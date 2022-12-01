#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>

#include "redis.h"
#include "json.hpp"
#include "offlinemsgmodel.h"
#include "friendmodel.h"
#include "usermodel.h"
#include "groupmodel.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
// 处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;

// 聊天服务器业务类
class ChatService {
public:
  // 获取 ChatService 单例对象
  static ChatService *instance();
  // 处理注册业务
  void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 处理登录业务
  void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 处理退出业务
  void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 一对一聊天业务
  void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 添加好友业务
  void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 创建群组业务
  void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 加入群组业务
  void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 群组聊天业务
  void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // redis 上报消息时的回调函数
  void handleRedisSubscribeMsg(int userId, string msg);
  // 获取消息对应的处理器 Handler
  MsgHandler getHandler(int msgid);
  // 处理客户端异常退出
  void clientCloseException(const TcpConnectionPtr &conn);
  // 服务器异常时业务重置方法
  void reset();
private:
  ChatService();
  // 储存消息 id <==> 对应业务的处理方法
  unordered_map<int, MsgHandler> msgHandlerMap_;
  // 存储在线用户的连接 id <==> connptr，服务器用来推送别的用户发给其他用户的聊天信息
  // 使用时应注意线程安全问题，比如多个用户同时登录同一个账号
  unordered_map<int, TcpConnectionPtr> userConnMap_;
  // 互斥锁 保证 userConnMap_ 线程安全
  mutex connMutex_;

  // 数据操作类对象
  UserModel userModel_;
  OfflineMsgModel offlineMsgModel_;
  FriendModel friendModel_;
  GroupModel groupModel_;

  // redis 操作对象
  Redis redis_;
};




#endif