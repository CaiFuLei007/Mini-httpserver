
#include "base/socket.h"
#include "util/quill_log.h"

#include <sys/types.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int Socket::CreateFd()
{
    int fd = socket(AF_INET , SOCK_STREAM  ,0);
    return fd;
}

bool Socket::Bind(uint16_t port , const std::string& ip)
{
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    serv_addr.sin_port = htons(port);

    if(bind(socketfd_ , (const sockaddr*)&serv_addr , sizeof(serv_addr)) < 0)
    {
        return false;
    }

    QLOG_INFO("{} BIND SUCCESS : {}:{} " , socketfd_ , ip , port);
    return true;
}

bool Socket::Listen()
{
    if(listen(socketfd_ , kDefaultListenCapacity) < 0)
    {
        return false;
    }
    QLOG_INFO("{} LISTEN SUCCESS " , socketfd_);

    return true;
}

bool Socket::Connect(uint16_t port , const std::string &ip)
{
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    serv_addr.sin_port = htons(port);

    if(connect(socketfd_ , (const sockaddr*)&serv_addr , sizeof(serv_addr)) < 0)
    {
        return false;
    }

    QLOG_INFO("{} CONNETCT SUCCESS : {}:{} " , socketfd_ , ip , port);

    return true;
}

size_t Socket::Recv(std::string& buf)
{   
    char tmp[1024]{0};
    ssize_t ret = recv(socketfd_ , tmp , sizeof(buf) / sizeof(char) , 0);
    buf = tmp;
    
    return ret;
}

bool Socket::Send(const std::string& buf)
{
    ssize_t ret = send(socketfd_  , buf.c_str() , buf.size() , 0);
    if(ret < 0)
    {
        return false;
    }

    return true;
}

int Socket::Accept()
{
    int fd = accept(socketfd_ , nullptr , nullptr);
    return fd;
}

bool Socket::CreateServer(uint16_t port ,  const std::string& ip)
{
    socketfd_ = CreateFd();
    if(socketfd_ < 0)
    {
        QLOG_ERROR("SOCKET FD CREATE FAIL");
        return false;
    }

    if(!Bind(port , ip))
    {
        QLOG_ERROR("SOCKET FD BIND FAIL");
        return false;
    }

    if(!Listen())
    {
        QLOG_ERROR("SOCKET FD LISTEN FAIL");
        return false;
    }
    return true;
}

size_t Socket::RecvNoBlock(std::string& buf)
{
    char tmp[1024]{0};
    ssize_t ret = recv(socketfd_ , tmp , sizeof(tmp) / sizeof(char) , MSG_DONTWAIT);
    buf = tmp;
    return ret;
}

bool Socket::SendNoBlock(const std::string& buf)
{
    ssize_t ret = send(socketfd_ , buf.c_str() , buf.size() , MSG_DONTWAIT);
    if(ret < 0)
    {
        return false;
    }
    return true;
}

bool Socket::CreateClient(uint16_t port , const std::string &ip)
{
    socketfd_ = CreateFd();
    if(socketfd_ < 0)
    {
        QLOG_ERROR("SOCKET FD CREATE FAIL");
        return false;
    }
    if(!Connect(port , ip))
    {
        QLOG_ERROR("SOCKET FD BIND FAIL");
        return false;
    }
    return true;
}