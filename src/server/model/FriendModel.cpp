#include "FriendModel.h"
#include "db.h"

// 添加好友关系
void FriendModel::Insert(const int& _userId, const int& _friendId)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "insert into friend values(%d,%d)", _userId, _friendId);

    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}

// 返回用户好友列表
std::vector<User> FriendModel::Query(const int& _userId)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid = '%d'", _userId);

    std::vector<User> vec;

    MySQL mysql;

    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);

        if (res)
        {
            MYSQL_ROW row;

            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;

                user.SetId(std::stoi(row[0]));
                user.SetName(row[1]);
                user.SetState(row[2]);

                vec.push_back(user);
            }

            mysql_free_result(res);

            return vec;
        }
    }

    return vec;
}