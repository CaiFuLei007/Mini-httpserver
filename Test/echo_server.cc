#include "tcp_server.h"
#include <iostream>

int main()
{
    TcpServer server(8888);

    server.SetReadCallback([](std::shared_ptr<Connection> conn, std::shared_ptr<Buffer> buf) {
        std::string msg;
        buf->ReadAndPop(msg);
        conn->SendMessage(msg);
    });

    server.SetCloseCallback([](std::shared_ptr<Connection> conn) {
        std::cout << "Connection closed" << std::endl;
    });

    server.SetErrorCallback([](std::shared_ptr<Connection> conn) {
        std::cout << "Connection error" << std::endl;
    });

    server.SetThreadCount(3);
    server.SetSelfRelease(10);
    server.Run();

    return 0;
}
