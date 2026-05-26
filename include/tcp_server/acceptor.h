
#pragma once

/*
    - 将对网络套接字 和 Channel 进行组合
    - 成员 : 
            1) Socket
            2) Channel
            3) 接收到新连接的回调
    - 接口 :
            1) socket 读事件就绪的回调
            2) Listen


*/

#include "base/socket.h"

#include <iostream>
#include <memory>
#include <functional>

class Channel;
class EventLoop;

class Acceptor
{
    using NewConnectCallback = std::function<void(int)>;
    using ConnectEventCallback = std::function<void()>;
private:
    Socket socket_;
    std::shared_ptr<Channel> channel_;
    NewConnectCallback new_connect_cb_;

    std::weak_ptr<EventLoop> eventloop_;

private:
    void AcceptReadCallback();

public:
    Acceptor(uint16_t port, std::shared_ptr<EventLoop> eventloop);

    void SetNewConnectCallback(NewConnectCallback cb)
    {
        new_connect_cb_ = cb;
    }

    void Listen();
};