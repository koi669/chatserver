#pragma once

#include <iostream>
#include <string>
#include <vector>

// 提高离线消息表的方法
class OfflineMsgModel
{
public:

    // 存储用户的离线消息
    void Insert(const int& _userId, const std::string& _msg);

    // 删除用户的离线消息
    void Remove(const int& _userId);

    // 查询用户的离线消息
    std::vector<std::string> Query(const int& _userId);
};