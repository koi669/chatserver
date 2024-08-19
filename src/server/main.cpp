#include <iostream>
#include <signal.h>

#include "ChatService.h"
#include "ChatServer.h"

// 处理服务器Ctrl + C结束后，重置user的状态信息
void ResetHandler(int)
{
    ChatService::GetInstance()->Reset();

    exit(0);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, ResetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);

    ChatServer server(&loop, addr, "ChatServer");

    server.Start();
    loop.loop();

    return 0;
}