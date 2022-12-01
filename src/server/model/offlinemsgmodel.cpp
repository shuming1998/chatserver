#include "offlinemsgmodel.h"
#include "db.h"


void OfflineMsgModel::insert(int userId, string msg) {
  // 组装 sql 语句
  char sql[1024] = {0};
  sprintf(sql, "insert into offlinemessage values('%d', '%s')",
      userId, msg.c_str());

  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}

void OfflineMsgModel::remove(int userId) {
  // 组装 sql 语句
  char sql[1024] = {0};
  sprintf(sql, "delete from offlinemessage where userid='%d'", userId);

  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}

vector<string> OfflineMsgModel::query(int userId) {
  // 组装 sql 语句
  char sql[1024] = {0};
  sprintf(sql, "select message from offlinemessage where userid = %d", userId);

  vector<string> offlineMsgs;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        // 将该用户所有离线消息放入 vector 并返回
        offlineMsgs.emplace_back(row[0]);
      }
      mysql_free_result(res);
    }
  }
  return offlineMsgs;
}