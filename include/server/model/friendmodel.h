#ifndef FRIEND_MODEL_H
#define FRIEND_MODEL_H

#include "user.h"
#include <vector>
using namespace std;

// 维护好友信息的操作接口方法
class FriendModel {
public:
  // 添加好友
  void insert(int userId, int friendId);
  // 返回用户好友列表 friendid => name state  (通过对user表和friend表的联合查询)
  vector<User> query(int userId);
private:

};



#endif