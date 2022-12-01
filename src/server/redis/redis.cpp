#include "redis.h"
#include <iostream>

Redis::Redis() : publish_context_(nullptr), subscribe_context_(nullptr) {}

Redis::~Redis() {
  if (publish_context_ != nullptr) {
    redisFree(publish_context_);
  }
  if (subscribe_context_ != nullptr) {
    redisFree(subscribe_context_);
  }
}

// 连接 redis 服务器
bool Redis::connect() {
  // 负责 publish 消息的上下文连接
  publish_context_ = redisConnect("127.0.0.1", 6379);
  if (publish_context_ == nullptr) {
    cerr << "connect redis failed!\n";
    return false;
  }
  // 负责 subscribe 消息的上下文连接
  subscribe_context_ = redisConnect("127.0.0.1", 6379);
  if (subscribe_context_ == nullptr) {
    cerr << "connect redis failed!" << '\n';
    return false;
  }

  // 在单独的线程中监听 channel 上的事件，因为 subscribe 是阻塞的，有消息时上报给业务层
  thread t([&]() { observer_channel_message(); });
  t.detach();
  cout << "connect redis-server success!\n";
  return true;
}

// 向 redis 指定通道 channel 发布消息
// redisCommand 就是 redisAppendCommand + redisBufferWrite + redisReply
// 但是 subscribe 会阻塞在 redisReply 以等待相应
bool Redis::publish(int channel, string message) {
  // 相当于在 redis-cli 中 $ publish [channel] "message"
  redisReply *reply = (redisReply *)redisCommand(publish_context_,
                       "PUBLISH %d %s", channel, message.c_str());
  if (reply == nullptr) {
    cerr << "publish command failed!\n";
    return false;
  }
  freeReplyObject(reply);
  return true;
}

// 向 redis 指定的通道 subscribe 订阅消息
bool Redis::subscribe(int channel) {
  // SUBSCRIBE 命令会造成线程阻塞等待通道中发生消息，所以在此只订阅通道，不接收通道消息
  // 通道消息的接收专门在 observer_channel_message 方法中通过独立的线程执行
  // 只负责发送命令，不阻塞接收 redis-server 响应消息，否则就会和 notifyMsg 线程抢占响应资源
  if (REDIS_ERR == redisAppendCommand(this->subscribe_context_, "SUBSCRIBE %d", channel)) {
    cerr << "subscribe command failed!\n";
    return false;
  }
  // redisBufferWrite 可以循环发送缓冲区，直到缓冲区数据发送完毕(done 被置为 1)
  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(this->subscribe_context_, &done)) {
      cerr << "subscribe command failed!\n";
      return false;
    }
  }
  return true;
}

// 向 redis 指定的通道 unsubscribe 订阅消息
bool Redis::unsubscribe(int channel) {
  if (REDIS_ERR == redisAppendCommand(this->subscribe_context_, "UNSUBSCRIBE %d", channel)) {
    cerr << "unsubscribe command failed!\n";
    return false;
  }

  // redisBufferWrite 可以循环发送缓冲区，直到缓冲区数据发送完毕(done 被置为 1)
  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(this->subscribe_context_, &done)) {
      cerr << "subscribe command failed!\n";
      return false;
    }
  }
  return true;
}

// 在独立的线程中接收订阅通道中的消息
void Redis::observer_channel_message() {
  redisReply *reply = nullptr;
  while (REDIS_OK == redisGetReply(this->subscribe_context_, (void **)&reply)) {
    // 订阅收到的消息是一个带三元素的数据 ["message","channel","messages"]
    if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
      // 给业务层上报通道上发生的消息
      notify_message_handler_(atoi(reply->element[1]->str), reply->element[2]->str);
    }
    freeReplyObject(reply);
  }
  cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<\n";
}

// 初始化向业务层上报通道消息的回调对象
void Redis::init_notify_handler(function<void(int, string)> fn) {
  this->notify_message_handler_ = fn;
}