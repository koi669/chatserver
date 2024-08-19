/*
muduo网络库给用户提供了两个主要的类
TcpServer: 用于编写服务器程序
TcpClient: 用于编写客户端程序

epoll + 线程池
好处：能够把网络I/O的代码和业务代码区分开
            -用户的连接和断开 用户的可读写事件
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

#include "json.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;
using json = nlohmann::json;

/* 基于muduo网络库开发服务器程序步骤

1、组合TcpServer对象
2、创建EventLoop事件循环对象的指针
3、明确TcpServer构造函数需要什么参数，输出
4、在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
5、设置合适的服务端线程数量，muduo库会自己分配I/O线程和工作线程

*/
class ChatServer
{
public:
    ChatServer(EventLoop* _loop, const InetAddress& _listenAddr, const std::string& _nameArg)
        : server_(_loop, _listenAddr, _nameArg), loop_(_loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        server_.setConnectionCallback(std::bind(&ChatServer::OnConnection, this, _1));

        // 给服务器注册用户对象而事件回调
        server_.setMessageCallback(std::bind(&ChatServer::OnMessage, this, _1, _2, _3));

        // 设置服务器端的线程数量   1个I/O线程（处理用户连接），3个工作线程
        server_.setThreadNum(4);
    }

    // 开启事件循环
    void Start()
    {
        server_.start();
    }

private:
    // 专门处理用户的连接创建和断开 epoll listen accept
    // 当有用户的连接创建和断开时会调用这个函数
    void OnConnection(const TcpConnectionPtr& _conn)
    {
        // peerAddress: 对端地址
        // localAddress: 本地地址
        if (_conn->connected())
            std::cout << _conn->peerAddress().toIpPort() << "->" << _conn->localAddress().toIpPort()
            << "连接成功" << std::endl;
        else
        {
            std::cout << _conn->peerAddress().toIpPort() << "->" << _conn->localAddress().toIpPort()
                << "连接失败" << std::endl;

            _conn->shutdown(); // close(fd);
        }
    }

    // 参数一：连接
    // 参数二：缓冲区
    // 参数三：时间
    // 当该连接有数据进来时调用这个函数
    void OnMessage(const TcpConnectionPtr& _conn, Buffer* _buf, Timestamp _time)
    {
        std::string buf = _buf->retrieveAllAsString();

        json js = json::parse(buf);

        std::cout << "recv data: " << js["from"] << " time: " << _time.toString() << std::endl;

        _conn->send(buf);
    }

    TcpServer server_; // #1

    EventLoop* loop_; // #2
};

int main()
{
    EventLoop loop;

    InetAddress addr("127.0.0.1", 6000);

    ChatServer server(&loop, addr, "ChatServer");

    server.Start();

    // 相当于epoll_wait，以阻塞的方式等待新用户连接，或已连接用户的读写事件
    loop.loop();

    return 0;
}