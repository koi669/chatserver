#include "OfflineMessageModel.h"
#include "db.h"

// 存储用户的离线消息
void OfflineMsgModel::Insert(const int& _userId, const std::string& _msg)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "insert into offlinemessage values('%d','%s')",
        _userId, _msg.c_str());

    MySQL mysql;

    if (mysql.connect())
        mysql.update(sql);

    return;
}

// 删除用户的离线消息
void OfflineMsgModel::Remove(const int& _userId)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "delete from offlinemessage where userid=%d", _userId);

    MySQL mysql;

    if (mysql.connect())
        mysql.update(sql);

    return;
}

// 查询用户的离线消息
std::vector<std::string> OfflineMsgModel::Query(const int& _userId)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "select message from offlinemessage where userid=%d", _userId);

    std::vector<std::string> vec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);

        if (res)
        {
            // 把_userid用户的所有离散消息放入vec中返回
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
                vec.push_back(row[0]);

            mysql_free_result(res);

            return vec;
        }
    }

    return vec;
}