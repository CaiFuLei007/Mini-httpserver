
#pragma once

/*
    - 对 Socket 套接字进行封装
    - 成员 :
            1) 套接字
    - 接口 :
            1) 绑定
            2) 监听
            3) 建立连接
            4) 接收, 发送消息
            5) 创建服务端
            6) 创建客户端
*/

#include <iostream>
#include <string>
#include <unistd.h>

class Socket
{
    static constexpr size_t kDefaultListenCapacity = 1024;
private:
    int socketfd_;

private:
    int CreateFd();
public:
    Socket()
    :socketfd_(-1)
    {}

    Socket(int fd)
    :socketfd_(fd)
    {}

    ~Socket()
    {
        close(socketfd_);
    }

    int Fd()
    {
        return socketfd_;
    }

    bool Bind(uint16_t port , const std::string& ip = "0.0.0.0");
    bool Listen();
    bool Connect(uint16_t port , const std::string &ip);
    int Accept();
    size_t Recv(std::string& buf);
    bool Send(const std::string& buf);

    ssize_t RecvNoBlock(std::string& buf);
    ssize_t SendNoBlock(const std::string& buf);

    bool CreateServer(uint16_t port ,  const std::string& ip = "0.0.0.0");
    bool CreateClient(uint16_t port , const std::string &ip);

    void ReusePort();
    void ReuseAddr();
};