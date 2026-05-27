

#pragma once


/*
    - 对连接进行管理
    - 成员 :
            1) 连接 ID
            2) Channel
            3) eventloop
            4) 6个回调 , 读回调 , 写回调 , 错误回调 , 任意回调(用户设置的) , 服务器关闭连接的回调
            5) 两个缓冲区 , 接收数据
            6) 连接对应的 Socket
            7) 连接是否设置定时销毁 , 以及定时销毁事件
            8) 连接的上下文
    - 接口 :
            1) 四个新连接的回调
            2) 从连接发送数据的接口

*/

#include "base/buffer.h"
#include "base/socket.h"

#include <iostream>
#include <memory>
#include <functional>
#include <any>

class Channel;
class EventLoop;

enum class ConnectionStatus
{
    CONNECTING, 
    CONNECTED , 
    DISCONNECTING , 
    DISCONNECTED
};


class Connection : public std::enable_shared_from_this<Connection>
{
    using MessageCallback = std::function<void(std::shared_ptr<Connection> , std::shared_ptr<Buffer> buf)>;
    using NewConnectCallback = std::function<void(std::shared_ptr<Connection> )>;
    using EventCallback = std::function<void(std::shared_ptr<Connection> )>;
    using CloseCallback = std::function<void(std::shared_ptr<Connection> )>;
    using SrvCloseCallback = std::function<void(std::shared_ptr<Connection> )>;
private:
    uint64_t conn_id_;
    std::shared_ptr<EventLoop> eventloop_;
    std::shared_ptr<Channel> channel_;
    
    MessageCallback message_callback_;
    NewConnectCallback newconnect_callback_;
    EventCallback event_callback_;
    CloseCallback close_callback_;
    SrvCloseCallback svr_close_callback_;

    std::shared_ptr<Buffer> in_buffer_;
    std::shared_ptr<Buffer> out_buffer_;

    Socket socket_;
    bool is_selfrelease_;
    uint16_t timeout_;

    enum ConnectionStatus status_;

    std::any context_;
private:
    void ReleaseInLoop();

    void ConnReadCallback();
    void ConnWriteCallback();
    void ConnErrorCallback();
    void ConnCloseCallback();
    void ConnEventCallback();

    void SendMessageInLoop(const std::string& message);

    void ReadyInLoop();
public:
    Connection(uint64_t conn_id , std::shared_ptr<EventLoop> eventloop , int fd);

    void SendMessage(const std::string& message);

    int Id()
    {
        return conn_id_;
    }

    void SetMessageCallback(MessageCallback cb)
    {   
        message_callback_ = cb;
    }
    void SetNewConnectCallback(NewConnectCallback cb)
    {   
        newconnect_callback_ = cb;
    }
    void SetEventCallback(EventCallback cb)
    {   
        event_callback_ = cb;
    }
    void SetCloseCallback(CloseCallback cb)
    {   
        close_callback_ = cb;
    }
    void SetSvrCloseCallback(CloseCallback cb)
    {   
        svr_close_callback_ = cb;
    }
    void Release();

    void SetSelfRelease(uint16_t timeout)
    {
        is_selfrelease_ = true;
        timeout_ = timeout;
    }

    void SetContext(const std::any context)
    {
        context_ = context;
    }

    std::any& GetContext()
    {
        return context_;
    }

    void Ready();
};