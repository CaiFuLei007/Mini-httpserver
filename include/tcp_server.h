
#pragma once

/*
    - 搭建 TCP 服务器
    - 成员 : 
            1) Acceptor + EventLoop 作为 主Reactor
            2) LoopThreadPool 从属 Reactor
            3) 哈希表 : 管理所有连接
            4) 5个回调 : 用户设置 读事件 , 写事件 , 错误事件 , 连接关闭事件 , 任意事件 , 新连接创建事件
            5) next_id 管理 conn_id 和 task_id
            6) 是否对长时间无消息连接进行自动释放
    - 接口 :
            1) 用户设置5个回调
            2) 设置从属线程个数
            3) Acceptor 回调
            4) 启动
            5) 设置 更新 取消定时任务
*/

#include "server/eventloop.h"
#include "server/acceptor.h"
#include "server/loopthread_poll.h"
#include "connection.h"

class TcpServer
{
    using ReadCallback = std::function<void(std::shared_ptr<Connection> , std::shared_ptr<Buffer> buf)>;
    using WriteCallback = std::function<void(std::shared_ptr<Connection> )>;
    using ErrorCallback = std::function<void(std::shared_ptr<Connection> )>;
    using EventCallback = std::function<void(std::shared_ptr<Connection> )>;
    using CloseCallback = std::function<void(std::shared_ptr<Connection> )>;

    using Task = std::function<void()>;
private:
    std::shared_ptr<EventLoop> eventloop_;
    Acceptor acceptor_;
    LoopThreadPoll threadpoll_;
    std::unordered_map<uint64_t , std::shared_ptr<Connection> > connections_;

    ReadCallback read_callback_;
    WriteCallback write_callback_;
    ErrorCallback error_callback_;
    EventCallback event_callback_;
    CloseCallback close_callback_;

    uint64_t next_id_;
    bool is_selfrelease_;
    uint16_t timeout_;

private:
    void CloseConnection(std::shared_ptr<Connection> conn);

    void AcceptCallback(int fd);

public:
    TcpServer(uint16_t port);

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

    void SetThreadCount(size_t count)
    {
        threadpoll_.SetCount(count);
    }

    void SetSelfRelease(uint16_t timeout)
    {
        is_selfrelease_ = true;
        timeout_ = timeout;
    }
    
    void Run();

    void SetScheduledTasks(size_t timeout ,Task task);
    void UpdateScheduledTasks(size_t id);
    void RemoveScheduledTasks(size_t id);
};