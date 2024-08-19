#pragma once

#include "user.hpp"

// User表的数据操作类
class UserModel
{
public:

    // User表的增加方法
    bool Insert(User& _user);

    // 根据用户号码查询用户信息
    User Query(const int& _id);

    // 更新用户的状态信息
    bool UpdateState(User _user);

    // 重置用户的状态信息
    void ResetState();
};