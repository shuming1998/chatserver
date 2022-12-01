#include "db.h"
#include "friendmodel.h"

void FriendModel::insert(int userId, int friendId) {
  // 组装 sql 语句
  char sql[1024] = {0};
  sprintf(sql, "insert into friend values('%d', '%d')",
      userId, friendId);

  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}

vector<User> FriendModel::query(int userId) {
  // 组装 sql 语句
  char sql[1024] = {0};
  // 联合查询 user 表和 friend 表
  sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b \
  on b.friendid = a.id where b.userid = %d", userId);

  vector<User> friendMsgs;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        User user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setState(row[2]);
        friendMsgs.emplace_back(user);
      }
      mysql_free_result(res);
    }
  }
  return friendMsgs;
}