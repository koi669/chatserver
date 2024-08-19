#include "UserModel.h"
#include "db.h"

#include <iostream>

// User表的增加方法
bool UserModel::Insert(User& _user)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "insert into user(name, password, state) values('%s','%s','%s')",
        _user.GetName().c_str(), _user.GetPwd().c_str(), _user.GetState().c_str());

    MySQL mysql;

    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            _user.SetId(mysql_insert_id(mysql.GetConnection()));
            return true;
        }
    }

    return false;
}

User UserModel::Query(const int& _id)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "select * from user where id = '%d'", _id);

    MySQL mysql;

    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (!res)
            return User();

        MYSQL_ROW row = mysql_fetch_row(res);

        if (!row)
            return User();

        User user;

        user.SetId(std::stoi(row[0]));
        user.SetName(row[1]);
        user.SetPwd(row[2]);
        user.SetState(row[3]);

        mysql_free_result(res);

        return user;
    }

    return User();
}

// 更新用户的状态信息
bool UserModel::UpdateState(User _user)
{
    // 组装Sql语句
    char sql[1024] = { 0 };
    std::sprintf(sql, "update user set state = '%s' where id = '%d'", _user.GetState().c_str(), _user.GetId());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            _user.SetId(mysql_insert_id(mysql.GetConnection()));

            return true;
        }
    }

    return false;
}

// 重置用户的状态信息
void UserModel::ResetState()
{
    // 组装Sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    MySQL mysql;

    if (mysql.connect())
        mysql.update(sql);
}