
#pragma once

/*
    - 对所有套接字 进行管理
    - 成员 : 
            1) 文件描述符
            2) 监听事件 , 就绪事件
            3) 事件回调 : 读事件 , 写事件 , 错误事件,  关闭事件
            4) eventloop 
    - 接口 : 
            1) 获取设置监听事件 , 就绪事件
            2) 设置回调
            3) 设置/关闭 读事件, 写事件监听

*/


#include <iostream>
#include <functional>
#include <unistd.h>
#include <memory>

class EventLoop;


class Channel : public std::enable_shared_from_this<Channel>
{
    using Callback = std::function<void()>;
private:
    int fd_;
    int events_;
    int revents_;
    Callback read_callback_;
    Callback write_callback_;
    Callback error_callback_;
    Callback close_callback_;
    Callback event_callback_;

    std::weak_ptr<EventLoop> eventloop_;

private:
    void Update();
    void Remove();

public:
    Channel(int fd , std::shared_ptr<EventLoop> eventloop)
    :fd_(fd) , 
    events_(0) ,
    revents_(0) , 
    eventloop_(eventloop)
    {}

    ~Channel()
    {
        close(fd_);
    }

    int Fd()
    {
        return fd_;
    }

    int GetEvents()
    {
        return events_;
    }

    void SetEvents(int events)
    {   
        events_ = events;
    }

    int GetRevents()
    {
        return revents_;
    }

    void SetRevents(int revents)
    {
        revents_ =revents;
    }

    void SetReadCallback(Callback cb)
    {
        read_callback_ = cb;
    }

    void SetWriteCallback(Callback cb)
    {
        write_callback_ = cb;
    }

    void SetErrorCallback(Callback cb)
    {
        error_callback_ = cb;
    }

    void SetEventCallback(Callback cb)
    {
        event_callback_ = cb;
    }

    void SetCloseCallback(Callback cb)
    {
        close_callback_ = cb;
    }

    void Handle();   // 处理就绪事件

    void ReadAble();
    void WriteAble();
    void UnReadAble();
    void UnWriteAble();

    void RemoveAllEvent();
};