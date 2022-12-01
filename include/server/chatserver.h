#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer {
public:
  // 初始化 ChatServer 对象
  ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);
  ~ChatServer() = default;

  // 启动服务
  void start();

private:
  // 连接事件回调函数
  void onConnection(const TcpConnectionPtr &);
  // 读写事件回调函数
  void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

  TcpServer server_;  // 服务器类对象
  EventLoop *loop_;   // 事件循环对象指针
};


















#endif