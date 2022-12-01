#include "groupmodel.h"
#include "db.h"

// 创建群组
bool GroupModel::creatGroup(Group &group) {
  // 组装 sql 语句
  char sql[1024] = {0};
  sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')",
          group.getName().c_str(), group.getDesc().c_str());
  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      // 创建后通过引用变量，将mysql生成的群组 id 写入到 group 中
      group.setId(mysql_insert_id(mysql.getConnection()));
      return true;
    }
  }
  return false;
}

// 加入群组
void GroupModel::addGroup(int userId, int groupId, string role) {
  // 组装 sql 语句
  char sql[1024] = {0};
  sprintf(sql, "insert into groupuser values('%d', '%d', '%s')",
          groupId, userId, role.c_str());
  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}

// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userId) {
  // 先根据 userId 在 groupuser 表中查出该用户所属的群组信息
  // 再根据群组信息，查询属于该群组的所有用户的 userid，并和user表进行多表联合查询，查出用户详细信息
  char sql[1024] = {0};
  sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join \
      groupuser b on a.id = b.groupid where b.userid=%d", userId);
  vector<Group> groupMsgs;

  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        Group group;
        group.setId(atoi(row[0]));
        group.setName(row[1]);
        group.setDesc(row[2]);
        groupMsgs.emplace_back(group);
      }
    }
  }

  // 查询群组的用户信息
  for (Group &group : groupMsgs) {
    sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a \
            inner join groupuser b on b.userid = a.id where b.groupid=%d", group.getId());
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        GroupUser user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setState(row[2]);
        user.setRole(row[3]);
        group.getUsers().emplace_back(user);
      }
      mysql_free_result(res);
    }
  }
  return groupMsgs;
}

// 根据指定群组 id 查询除了 userId 之外的用户 id 列表，用于给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userId, int groupId) {
  char sql[1024] = {0};
  sprintf(sql, "select userid from groupuser where groupid = %d \
          and userid != %d", groupId, userId);

  vector<int> users;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res))) {
        users.emplace_back(atoi(row[0]));
      }
    }
  }
  return users;
}