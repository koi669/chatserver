#include <muduo/base/Logging.h>
#include <string>
#include <vector>
#include <iostream>
#include <map>

#include "ChatService.h"
#include "public.h"

using namespace muduo;

// 获取单例对象的接口函数
ChatService* ChatService::GetInstance()
{
    static ChatService service;

    return &service;
}

// 注册消息以及对应的handler回调
ChatService::ChatService()
{
    msgHandlerMap_.insert({ LOGIN_MSG, std::bind(&ChatService::Login, this, _1, _2, _3) });
    msgHandlerMap_.insert({ REG_MSG, std::bind(&ChatService::Region, this, _1, _2, _3) });
    msgHandlerMap_.insert({ ONE_CHAT_MSG, std::bind(&ChatService::OneChat, this, _1, _2, _3) });
    msgHandlerMap_.insert({ ADD_FRIEND_MSG, std::bind(&ChatService::AddFriend, this, _1, _2, _3) });

    msgHandlerMap_.insert({ CREATE_GROUP_MSG, std::bind(&ChatService::CreateGroup, this, _1, _2, _3) });
    msgHandlerMap_.insert({ ADD_GROUP_MSG, std::bind(&ChatService::AddGroup, this, _1, _2, _3) });
    msgHandlerMap_.insert({ GROUP_CHAT_MSG, std::bind(&ChatService::GroupChat, this, _1, _2, _3) });

    msgHandlerMap_.insert({ LOGINOUT_MSG, std::bind(&ChatService::LoginOut, this, _1, _2, _3) });

    // 连接redis服务器
    if (redis_.connect())
        // 设置上报消息的回调
        redis_.init_notify_handler(std::bind(&ChatService::HandleRedisSubscribeMessage, this, _1, _2));
}

MsgHandler ChatService::GetHandler(int _msgId)
{
    // 记录错误日志，_msgId没有对应的事件处理回调
    if (msgHandlerMap_.find(_msgId) == msgHandlerMap_.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
            {
                LOG_ERROR << "msgid: " << _msgId << " can not find handler";
            };
    }

    return msgHandlerMap_[_msgId];
}

// 处理登录业务
void ChatService::Login(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    // LOG_INFO << "do login service!!!";
    int id = _js["id"].get<int>();
    std::string pwd = _js["password"];

    User user = userModel_.Query(id);
    if (user.GetId() == id && user.GetPwd() == pwd)
    {
        if (user.GetState() == "online")
        {
            // 用户已经登录，不允许重复登录
            json response;

            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账户已经登录，请重新输入新账号";

            _conn->send(response.dump());
        }
        else
        {
            {
                std::lock_guard<std::mutex> lock(connMutex_);

                // 登录成功，记录用户连接信息
                userConnMap_.insert({ id, _conn });
            }

            // 登录成功后，向redis订阅channel(id)
            redis_.subscribe(id);

            // 登录成功，更新用户状态信息 state: offline -》online
            user.SetState("online");
            userModel_.UpdateState(user);

            json response;

            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.GetId();
            response["name"] = user.GetName();

            // 查询该用户是否有离线消息
            std::vector<std::string> vec = offlineMsgModel_.Query(id);
            if (vec.size() > 0)
            {
                response["offlinemsg"] = vec;

                // 读取该用户的离线消息后，把该用户的所有离线消息删除
                offlineMsgModel_.Remove(id);
            }

            // 查询该用户的好友信息并返回
            std::vector<User> userVec = friendModel_.Query(id);
            if (userVec.size() > 0)
            {
                std::vector<std::string> vec2;

                for (User& user : userVec)
                {
                    json js;

                    js["id"] = user.GetId();
                    js["name"] = user.GetName();
                    js["state"] = user.GetState();

                    vec2.push_back(js.dump());
                }

                response["friends"] = vec2;
            }

            // 查询该用户的群组消息并返回
            std::vector<Group> groupVec = groupModel_.queryGroups(id);
            if (groupVec.size() > 0)
            {
                std::vector<std::string> vec3;

                for (Group& group : groupVec)
                {
                    json groupJson;

                    groupJson["id"] = group.getId();
                    groupJson["groupname"] = group.getName();
                    groupJson["groupdesc"] = group.getDesc();

                    std::vector<std::string> userV;

                    for (GroupUser& user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.GetId();
                        js["name"] = user.GetName();
                        js["state"] = user.GetState();
                        js["role"] = user.GetRole();

                        userV.push_back(js.dump());
                    }

                    groupJson["users"] = userV;

                    vec3.push_back(groupJson.dump());
                }

                response["groups"] = vec3;
            }

            _conn->send(response.dump());
        }
    }
    else
    {
        // 该用户不存在，登录失败
        json response;

        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名密码或者错误";

        _conn->send(response.dump());
    }
}

// 处理注册业务
void ChatService::Region(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    // LOG_INFO << "do reg service!!!";
    std::string name = _js["name"];
    std::string pwd = _js["password"];

    User user;
    user.SetName(name);
    user.SetPwd(pwd);

    bool state = userModel_.Insert(user);
    if (state)
    {
        // 注册成功
        json response;

        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.GetId();

        _conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;

        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["id"] = user.GetId();

        _conn->send(response.dump());
    }
}

// 添加好友业务 msgId id friendid
void ChatService::AddFriend(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    int userid = _js["id"].get<int>();
    int friendid = _js["friendid"].get<int>();

    // 存储好友信息
    friendModel_.Insert(userid, friendid);
}

// 服务器异常后，业务重置方法
void ChatService::Reset()
{
    // 把online状态的用户，设置成offline
    userModel_.ResetState();
}

void ChatService::LoginOut(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    int userid = _js["id"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connMutex_);

        auto it = userConnMap_.find(userid);

        if (it != userConnMap_.end())
            userConnMap_.erase(it);
    }

    // 用户注销，相当于下线，取消订阅
    redis_.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    userModel_.UpdateState(user);
}

// 处理客户端异常退出
void ChatService::ClientCloseException(const TcpConnectionPtr& _conn)
{
    User user;

    {
        std::lock_guard<std::mutex> lock(connMutex_);

        for (auto it = userConnMap_.begin();it != userConnMap_.end();it++)
        {
            if (it->second == _conn)
            {
                // 从map表删除用户的连接信息
                user.SetId(it->first);

                userConnMap_.erase(it);
                break;
            }
        }
    }

    // 用户注销，相当于下线，取消订阅
    redis_.unsubscribe(user.GetId());

    // 更新用户的状态信息
    if (user.GetId() == -1)
        return;

    user.SetState("offline");
    userModel_.UpdateState(user);
}

// 一对一聊天业务
void ChatService::OneChat(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    int toId = _js["toid"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connMutex_);

        if (userConnMap_.find(toId) != userConnMap_.end())
        {
            // toId在线，转发消息
            userConnMap_[toId]->send(_js.dump());

            return;
        }
    }

    // 查询toId是否在线
    User user = userModel_.Query(toId);
    if (user.GetState() == "online")
    {
        redis_.publish(toId, _js.dump());
        return;
    }

    // toId不在线，存储离线消息
    offlineMsgModel_.Insert(toId, _js.dump());
}

// 创建群组业务
void ChatService::CreateGroup(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    int userid = _js["id"].get<int>();
    std::string name = _js["groupname"];
    std::string desc = _js["groupdesc"];

    // 存储新创建的群组消息
    Group group(-1, name, desc);
    if (groupModel_.createGroup(group))
        groupModel_.addGroup(userid, group.getId(), "creator");
}

// 加入群组业务
void ChatService::AddGroup(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    int userid = _js["id"].get<int>();
    int groupid = _js["groupid"].get<int>();

    groupModel_.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::GroupChat(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)
{
    int userid = _js["id"].get<int>();
    int groupid = _js["groupid"].get<int>();

    std::vector<int> useridVec = groupModel_.queryGroupUsers(userid, groupid);

    std::lock_guard<std::mutex> lock(connMutex_);

    for (int id : useridVec)
    {
        auto it = userConnMap_.find(id);

        if (it == userConnMap_.end())
        {
            // 查询toId是否在线
            User user = userModel_.Query(id);

            if (user.GetState() == "online")
                redis_.publish(id, _js.dump());
            else
                offlineMsgModel_.Insert(id, _js.dump()); // 存储离线消息
        }
        else
            it->second->send(_js.dump());

    }

}

void ChatService::HandleRedisSubscribeMessage(int _userid, string _msg)
{
    std::lock_guard<std::mutex> lock(connMutex_);

    auto it = userConnMap_.find(_userid);

    if (it != userConnMap_.end())
    {
        it->second->send(_msg);

        return;
    }

    // 存储该用户的离线消息
    offlineMsgModel_.Insert(_userid, _msg);
}