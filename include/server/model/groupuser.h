#ifndef GROUP_USER_H
#define GROUP_USER_H

#include "user.h"

class GroupUser : public User
{
public:
  void setRole(string role) { role_ = role; }
  string getRole() { return role_; }

private:
  string role_;
};







#endif
