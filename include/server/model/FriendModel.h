#pragma once

#include <vector>

#include "user.hpp"

// 维护好友信息表的操作接口方法
class FriendModel
{
public:

    // 添加好友关系
    void Insert(const int& _userId, const int& _friendId);

    // 返回用户好友列表
    std::vector<User> Query(const int& _userId);
};