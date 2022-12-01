#ifndef GROUP_MODEL_H
#define GROUP_MODEL_H

#include "group.h"
#include <string>
#include <vector>
using namespace std;

class GroupModel {
public:
  // 创建群组
  bool creatGroup(Group &group);
  // 加入群组
  void addGroup(int userId, int groupId, string role);
  // 查询用户所在群组信息
  vector<Group> queryGroups(int userId);
  // 根据指定群组 id 查询除了 userId 之外的用户 id 列表，用于给群组其他成员群发消息
  vector<int> queryGroupUsers(int userId, int groupId);


private:




};








#endif