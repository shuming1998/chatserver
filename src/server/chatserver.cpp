#include <string>
#include <iostream>
#include <functional>

#include "chatserver.h"
#include "json.hpp"
#include "chatservice.h"

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    : server_(loop, listenAddr, nameArg) {
  server_.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
  server_.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
  server_.setThreadNum(7);
}

void ChatServer::start() {
  server_.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn) {
  // 客户端断开连接
  if (!conn->connected()) {
    // 处理客户端异常关闭
    ChatService::instance()->clientCloseException(conn);
    conn->shutdown();
  }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
  string buf = buffer->retrieveAllAsString();
  cout << buf << endl;
  // 消息反序列化
  json js = json::parse(buf);
  // 为了将网络模块和业务模块解耦，通过回调函数处理消息
  auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
  // 根据消息 id 从业务模块的 msgHandlerMap_ 中获取的对应的业务处理函数
  msgHandler(conn, js, time);
}

