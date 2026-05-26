
#pragma once

/*
    - 将 thread 和 eventloop 进行绑定
    - 分为两个部分 : LoopThread 单个线程 LoopThreadPoll 线程池
    - LoopThread
        - 成员 :
                1) eventloop
                2) thread
                3) 锁和条件变量
        - 接口 : 
                1) 获取 eventloop 接口
                2) 启动线程
    
    - LoopThreadPoll
        - 成员 :
                1) 线程个数
                2) 下一个 连接绑定的 eventloop

*/

#include "server/eventloop.h"

#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

class LoopThread
{
private:
    std::thread thread_;
    std::shared_ptr<EventLoop> eventloop_;

    std::mutex mutex_;
    std::condition_variable cond_;

private:
    void ThreadCallback();

public:
    LoopThread()
    {
    }

    std::shared_ptr<EventLoop> GetEventLoop();

    void Run();
};



class LoopThreadPoll
{
private:
    size_t count_;
    std::vector<std::shared_ptr<LoopThread> > loops_;
    std::vector<std::shared_ptr<EventLoop> > eventloops_;

    size_t next_num_;
public:
    LoopThreadPoll()
    :count_(0)
    {}

    void SetCount(size_t count)
    {
        count_ = count;
    }

    std::shared_ptr<EventLoop> NextEventLoop();
    void Run();

};