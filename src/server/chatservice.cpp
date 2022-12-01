#include <muduo/base/Logging.h>
#include <iostream>
#include <vector>

#include "chatservice.h"
#include "public.h"

using namespace muduo;
using namespace std;

// 注册消息以及对应的 Handler 回调操作
ChatService::ChatService() {
  msgHandlerMap_.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
  msgHandlerMap_.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
  msgHandlerMap_.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
  msgHandlerMap_.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
  msgHandlerMap_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

  msgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
  msgHandlerMap_.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
  msgHandlerMap_.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

  // 连接redis 服务器
  if (redis_.connect()) {
    cout << "<<<<<<<<<<<<<<<<<<<Redis Connect Success>>>>>>>>>>>>>>>>>\n";
    // 设置通道上有消息上报时的回调
    redis_.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMsg, this, _1, _2));
  }
}

ChatService *ChatService::instance() {
  static ChatService service;
  return &service;
}

MsgHandler ChatService::getHandler(int msgid) {
  // 如果 msgHandlerMap_ 中 没有 msgid 对应的事件处理回调，返回打印错误日志的处理器
  auto it = msgHandlerMap_.find(msgid);
  if (it == msgHandlerMap_.end()) {
    return [=](const TcpConnectionPtr &conn, json &js, Timestamp){ LOG_ERROR << "msgid: " << msgid << " can't find handler!"; };
  } else {
    return msgHandlerMap_[msgid];
  }
}

// 填入 name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  string name = js["name"];
  string pwd = js["password"];

  User user;
  user.setName(name);
  user.setPwd(pwd);
  bool state = userModel_.insert(user);
  if (state) {
    // 注册成功
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 0;
    response["id"] = user.getId();
    conn->send(response.dump());
  } else {
    // 注册失败
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    conn->send(response.dump());
  }
}

// id pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  int id = js["id"].get<int>();
  string pwd = js["password"];

  User user = userModel_.query(id);
  if (user.getId() == id && user.getPwd() == pwd) {
    // 重复登录
    if (user.getState() == "online") {
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 2;
      response["errmsg"] = "This account is using, input another!";
      conn->send(response.dump());
    } else {
      {
        // 登录成功，记录用户连接信息，加锁保证线程安全
        // 用代码块控制加锁粒度
        lock_guard<mutex> lock(connMutex_);
        userConnMap_.insert({id, conn});
      }

      // id 用户登录成功后，向 redis 订阅 channel(id)
      redis_.subscribe(id);

      // 登录成功，更新用户状态信息
      user.setState("online");
      userModel_.updateState(user);

      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 0;
      response["id"] = user.getId();
      response["name"] = user.getName();

      // 查询该用户是否有离线消息
      vector<string> offlineMsgs = offlineMsgModel_.query(id);
      if (!offlineMsgs.empty()) {
        // json 直接将 vector 序列化
        response["offlinemsg"] = offlineMsgs;
        // 清空读取后的离线消息
        offlineMsgModel_.remove(id);
      }

      // 查询该用户的好友信息并返回
      vector<User> friends = friendModel_.query(id);
      if (!friends.empty()) {
        vector<string> friendMsgs;
        for (User &user : friends) {
          json js;
          js["id"] = user.getId();
          js["name"] = user.getName();
          js["state"] = user.getState();
          friendMsgs.emplace_back(js.dump());
        }
        response["friends"] = friendMsgs;
      }

      // 查询用户的群组信息
      vector<Group> groupuserVec = groupModel_.queryGroups(id);
      if (!groupuserVec.empty()) {
        // group:[{groupid:[xxx, xxx, xxx, xxx]}]
        vector<string> groupV;
        for (Group &group : groupuserVec) {
          json grpjson;
          grpjson["id"] = group.getId();
          grpjson["groupname"] = group.getName();
          grpjson["groupdesc"] = group.getDesc();
          vector<string> userV;
          for (GroupUser &user : group.getUsers()) {
            json js;
            js["id"] = user.getId();
            js["name"] = user.getName();
            js["state"] = user.getState();
            js["role"] = user.getRole();
            userV.push_back(js.dump());
          }
          grpjson["users"] = userV;
          groupV.push_back(grpjson.dump());
        }
        response["groups"] = groupV;
      }
      conn->send(response.dump());
    }
  } else {
    // 密码错误
    if (user.getId() == id) {
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 1;
      response["errmsg"] = "invalid password!";
      conn->send(response.dump());
    } else {
      // 账号不存在
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 1;
      response["errmsg"] = "invalid id!";
      conn->send(response.dump());
    }
  }
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  int userId = js["id"].get<int>();
  string name = js["groupname"];
  string desc = js["groupdesc"];

  // 存储新创建的群组信息
  Group group(-1, name, desc);
  if (groupModel_.creatGroup(group)) {
    groupModel_.addGroup(userId, group.getId(), "creator");
  }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  int userId = js["id"].get<int>();
  int groupId = js["groupid"].get<int>();
  groupModel_.addGroup(userId, groupId, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  int userId = js["id"].get<int>();
  int groupId = js["groupid"].get<int>();
  // 查询该用户所在群组的所有其他用户的 id
  vector<int> usersId = groupModel_.queryGroupUsers(userId, groupId);

  lock_guard<mutex> lock(connMutex_);
  for (int id : usersId) {
    auto it = userConnMap_.find(id);
    if (it != userConnMap_.end()) {
      // 如果找到该用户的连接，直接转发群消息
      it->second->send(js.dump());
    } else {
      // 查询 id 是否在其他机器上在线，在线的话，通过 redis channel 发送消息
      User user = userModel_.query(id);
      if (user.getState() == "online") {
        redis_.publish(id, js.dump());
        return;
      }
      // 用户在任何地点都不在线，存储离线消息
      offlineMsgModel_.insert(id, js.dump());
    }
  }
}

// 从 redis 消息队列中获取订阅消息，事件发生时，由 redis 调用
// 用户所在的服务器会收到订阅消息，因为用户所在的服务器订阅时注册的 channel 是用户 id
void ChatService::handleRedisSubscribeMsg(int userId, string msg) {
  // 通过 userConnMap_ 将消息发送给该用户
  lock_guard<mutex> lock(connMutex_);
  auto it = userConnMap_.find(userId);
  if (it != userConnMap_.end()) {
    it->second->send(msg);
    return;
  }

  // 消息上报的过程中，该用户下线了，存储离线消息
  offlineMsgModel_.insert(userId, msg);
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  int userid = js["id"].get<int>();
  {
    lock_guard<mutex> lock(connMutex_);
    // 从下线的用户从本地的 userConnMap_ 中删除
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end()) {
        userConnMap_.erase(it);
    }
  }
  // 用户注销，相当于就是下线，在redis中取消订阅通道
  redis_.unsubscribe(userid);

  // 更新用户的状态信息
  User user(userid, "", "", "offline");
  userModel_.updateState(user);
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
  User user;
  {
    lock_guard<mutex> lock(connMutex_);
    // 从 userConnMap_ 中找到该用户对应的连接并删除
    for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it) {
      if (it->second == conn) {
        user.setId(it->first);
        userConnMap_.erase(it);
        break;
      }
    }
  }

  // 取消该用户在 redis 中订阅的 channel
  redis_.unsubscribe(user.getId());

  // 更新用户状态信息
  if (user.getId() != -1) {
    user.setState("offline");
    userModel_.updateState(user);
  }
}

// 服务器异常时业务重置方法
void ChatService::reset() {
  // 将所有 online 用户的状态设置为 offline
  userModel_.resetState();
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  int toId = js["toid"].get<int>();
  {
    lock_guard<mutex> lock(connMutex_);
    // 查找用户是否本地在线
    auto it = userConnMap_.find(toId);
    if (it != userConnMap_.end()) {
      // 消息接收方在线， 服务器直接转发消息
      it->second->send(js.dump());
      return;
    }
  }

  // 查询 toId 是否在其他机器上在线，在线的话，通过 redis channel 发送消息
  User user = userModel_.query(toId);
  if (user.getState() == "online") {
    redis_.publish(toId, js.dump());
    return;
  }

  // 确定消息接收方不在任何地点在线，存储离线消息
  offlineMsgModel_.insert(toId, js.dump());
}

// msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  int userId = js["id"].get<int>();
  int friendId = js["friendid"].get<int>();

  // 存储好友信息
  friendModel_.insert(userId, friendId);
}
