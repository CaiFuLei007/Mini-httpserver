

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
    - 接口 :
            1) 四个新连接的回调
            2) 从连接发送数据的接口

*/

#include "base/buffer.h"
#include "base/socket.h"

#include <iostream>
#include <memory>
#include <functional>

class Channel;
class EventLoop;

class Connection : public std::enable_shared_from_this<Connection>
{
    using ReadCallback = std::function<void(std::shared_ptr<Connection> , std::shared_ptr<Buffer> buf)>;
    using WriteCallback = std::function<void(std::shared_ptr<Connection> )>;
    using ErrorCallback = std::function<void(std::shared_ptr<Connection> )>;
    using EventCallback = std::function<void(std::shared_ptr<Connection> )>;
    using CloseCallback = std::function<void(std::shared_ptr<Connection> )>;
    using SrvCloseCallback = std::function<void(std::shared_ptr<Connection> )>;
private:
    uint64_t conn_id_;
    std::shared_ptr<EventLoop> eventloop_;
    std::shared_ptr<Channel> channel_;
    
    ReadCallback read_callback_;
    WriteCallback write_callback_;
    ErrorCallback error_callback_;
    EventCallback event_callback_;
    CloseCallback close_callback_;
    SrvCloseCallback svr_close_callback_;

    std::shared_ptr<Buffer> in_buffer_;
    std::shared_ptr<Buffer> out_buffer_;

    Socket socket_;
    bool is_selfrelease_;
    uint16_t timeout_;

private:
    void ReleaseInLoop();

    void ConnReadCallback();
    void ConnWriteCallback();
    void ConnErrorCallback();
    void ConnCloseCallback();
    void ConnEventCallback();

    void SendMessageInLoop(const std::string& message);
public:
    Connection(uint64_t conn_id , std::shared_ptr<EventLoop> eventloop , int fd);
    void SendMessage(const std::string& message);

    void SetReadCallback(ReadCallback cb)
    {   
        read_callback_ = cb;
    }
    void SetWriteCallback(WriteCallback cb)
    {   
        write_callback_ = cb;
    }
    void SetErrorCallback(ErrorCallback cb)
    {   
        error_callback_ = cb;
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

    void Ready();
};