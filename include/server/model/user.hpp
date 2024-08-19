#pragma once

#include <string>

// 匹配User表的ORM类
class User
{
public:

    User(int _id = -1, std::string _name = "", std::string _pwd = "", std::string _state = "offline")
    {
        this->id_ = _id;
        this->name_ = _name;
        this->passWord_ = _pwd;
        this->state_ = _state;
    }

    void SetId(const int& _id)
    {
        this->id_ = _id;
    }

    void SetName(const std::string& _name)
    {
        this->name_ = _name;
    }

    void SetPwd(const std::string& _pwd)
    {
        this->passWord_ = _pwd;
    }

    void SetState(const std::string& _state)
    {
        this->state_ = _state;
    }

    int GetId()
    {
        return this->id_;
    }

    std::string GetName()
    {
        return this->name_;
    }

    std::string GetPwd()
    {
        return this->passWord_;
    }

    std::string GetState()
    {
        return this->state_;
    }

private:

    int id_;
    std::string name_;
    std::string passWord_;
    std::string state_;

};