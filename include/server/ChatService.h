#pragma once

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>

#include "UserModel.h"
#include "json.hpp"
#include "OfflineMessageModel.h"
#include "FriendModel.h"
#include "groupmodel.hpp"
#include "group.hpp"
#include "redis.hpp"

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time)>;

// 聊天服务器业务类
class ChatService
{
public:

    // 获取单例模式的接口函数
    static ChatService* GetInstance();

    // 处理登录业务
    void Login(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    // 处理注册业务
    void Region(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    // 添加好友业务
    void AddFriend(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    // 获取消息对应的处理器
    MsgHandler GetHandler(int _msgId);

    // 服务器异常后，业务重置方法
    void Reset();

    // 处理客户端异常退出
    void ClientCloseException(const TcpConnectionPtr& _conn);

    // 一对一聊天业务
    void OneChat(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    // 处理注销业务
    void LoginOut(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    // 创建群组业务
    void CreateGroup(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    // 加入群组业务
    void AddGroup(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    // 群组聊天业务
    void GroupChat(const TcpConnectionPtr& _conn, json& _js, const Timestamp& _time);

    void HandleRedisSubscribeMessage(int _userid, string _msg);

private:

    // 构造函数私有化
    ChatService();

    // 存储消息id和其对应的业务处理方法
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    // 存储在线用户的通信连接
    std::unordered_map<int, TcpConnectionPtr> userConnMap_;

    // 定义互斥锁，保证userConnMap_的线程安全
    std::mutex connMutex_;

    // 数据操作类对象
    UserModel           userModel_;
    OfflineMsgModel     offlineMsgModel_;
    FriendModel         friendModel_;
    GroupModel          groupModel_;

    // redis操作对象
    Redis redis_;
};