#include <functional>
#include <string>

#include "json.hpp"
#include "ChatServer.h"
#include "ChatService.h"

using namespace std::placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* _loop, const InetAddress& _listenAddr, const string& _nameArg)
    : server_(_loop, _listenAddr, _nameArg)
    , loop_(_loop)
{
    // 注册连接回调
    server_.setConnectionCallback(std::bind(&ChatServer::OnConnectionCallBack, this, _1));

    // 注册消息回调
    server_.setMessageCallback(std::bind(&ChatServer::OnMessageCallBack, this, _1, _2, _3));

    // 设置线程数量
    server_.setThreadNum(4);
}

void ChatServer::Start()
{
    server_.start();
}

// 上报链接相关信息的回调函数
void ChatServer::OnConnectionCallBack(const TcpConnectionPtr& _conn)
{
    // 客户端断开连接
    if (!_conn->connected())
    {
        ChatService::GetInstance()->ClientCloseException(_conn);

        _conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::OnMessageCallBack(const TcpConnectionPtr& _conn, Buffer* _buffer, Timestamp _time)
{
    std::string buf = _buffer->retrieveAllAsString();

    // 数据的反序列化
    json js = json::parse(buf);

    // 达到的目的：完全解耦网络模块的代码和业务模块的代码
    // 通过js["msgId"] 获取-》业务handler
    auto msgHandler = ChatService::GetInstance()->GetHandler(js["msgid"].get<int>());

    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(_conn, js, _time);
}
