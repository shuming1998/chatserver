#ifndef OFFLINE_MSG_MODEL_H
#define OFFLINE_MSG_MODEL_H

#include <string>
#include <vector>
using namespace std;

// 提供离线消息表的接口方法
class OfflineMsgModel {
public:
  // 存储用户离线消息
  void insert(int userId, string msg);
  // 删除用户离线消息
  void remove(int userId);
  // 查询用户离线消息
  vector<string> query(int userId);


private:


};












#endif
