#include "chatserver.h"
#include "chatservice.h"
#include <signal.h>
#include <iostream>
using namespace std;

// 用来处理服务器端 ctrl+c，保证退出前重置 user 的状态
void resetHandler(int) {
  ChatService::instance()->reset();
  exit(0);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << '\n';
    exit(-1);
  }
  char *ip = argv[1];
  uint16_t port = atoi(argv[2]);
  signal(SIGINT, resetHandler);
  EventLoop loop;
  InetAddress addr(ip, port);
  ChatServer server(&loop, addr, "ChatServer");

  server.start();
  loop.loop();

  return 0;
}