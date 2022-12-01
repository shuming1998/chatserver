#ifndef USER_H
#define USER_H
#include <string>

using namespace std;

// 匹配 User 表的 ORM 类(对象关系映射框架，将业务模块和数据模块解耦)
class User {
public:
  User(int id = -1, string name = "", string pwd = "", string state = "offline")
      : id(id), name(name), password(pwd), state(state) {
  }

  void setId(int id) { this->id = id; }
  void setName(string name) { this->name = name; }
  void setPwd(string pwd) { this->password = pwd; }
  void setState(string state) { this->state = state; }

  int getId() { return this->id; }
  string getName() { return this->name; }
  string getPwd() { return this->password; }
  string getState() { return this->state; }

protected:
  int id;
  string name;
  string password;
  string state;

};


#endif