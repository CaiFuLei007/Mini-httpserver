

#pragma once

/*
    - EventLoop 事件循环 , 每个线程独占一个 , 循环等待处理事件
    - 成员 :
            1) Poller 
            2) eventfd 和 对应的 channel
            3) 任务队列 , 以及配对的 锁
            4) 定时器
            5) 线程 ID
    - 接口 : 
            1) 创建 eventfd , 对应的读事件和写事件
            2) 添加 , 更新 , 移除定时任务
            3) 处理所有任务
            4) RunInLoop , IsInLoop
            5) PutIntoQueue
*/

#include "base/poller.h"
#include "base/channel.h"
#include "tcp_server/timerwheel.h"

#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>


class EventLoop : public std::enable_shared_from_this<EventLoop>
{       
        using Task = std::function<void()>; 
private:
        std::thread::id id_;
        Poller poller_;
        int eventfd_;
        std::shared_ptr<Channel> channel_;
        std::vector<Task> tasks_;
        std::mutex mutex_;
        std::unique_ptr<TimerWheel> timerwheel_;

private:
        static int CreateEventFd();

        void EventFdReadCallback();

        bool AddTimedJobInLoop(size_t id , size_t timeout , Task task);
        bool UpdateTimedJobInLoop(size_t id);
        bool CancelTimedJobInLoop(size_t id);

        void NotifyThreadInLoop();

        void UpdateEventInLoop(std::shared_ptr<Channel> channel);
        void RemvoeEventInLoop(std::shared_ptr<Channel> channel);
public:
        EventLoop();
        ~EventLoop();
        
        void NotifyThread();

        bool AddTimedJob(size_t id , size_t timeout , Task task);
        bool UpdateTimedJob(size_t id);
        bool CancelTimedJob(size_t id);

        bool IsInLoop();
        void RunInLoop(Task task);
        void PutIntoQueue(Task task);      
        
        void UpdateEvent(std::shared_ptr<Channel> channel);
        void RemvoeEvent(std::shared_ptr<Channel> channel);

        void Init();

        void HanleTask();
}; 
