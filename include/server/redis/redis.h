#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

using namespace std;

//https://blog.csdn.net/QIANGWEIYUAN/article/details/97895611

class Redis {
public:
  Redis();
  ~Redis();

  // 连接 redis 服务器
  bool connect();

  // 向 redis 指定通道 channel 发布消息
  bool publish(int channel, string message);

  // 向 redis 指定的通道 subscribe 订阅消息
  bool subscribe(int channel);

  // 向 redis 指定的通道 unsubscribe 订阅消息
  bool unsubscribe(int channel);

  // 在独立的线程中接收订阅通道中的消息
  void observer_channel_message();

  // 初始化向业务层上报通道消息的回调对象
  void init_notify_handler(function<void(int, string)>fn);

private:
  //----一个上下文可以理解为一个 redis-cli 包含的东西，publish 和 subscribe 不能在一个上下文中处理(会阻塞)
  // hiredis 同步上下文对象，负责 publish 消息
  redisContext *publish_context_;
  // hiredis 同步上下文对象，负责 subscribe 消息
  redisContext *subscribe_context_;

  // 回调操作，收到订阅的消息，给 service 层上报  (通道号,消息)
  function<void(int, string)> notify_message_handler_;
};


#endif