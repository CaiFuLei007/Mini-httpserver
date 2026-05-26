
#include "tcp_server/timerwheel.h"
#include "base/util/quill_log.h"
#include "base/channel.h"

#include <signal.h>
#include <time.h>
#include <sys/timerfd.h>


int TimerWheel::CreateTimerFd()
{
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(fd < 0)
    {
        QLOG_ERROR("TIMERFD CREATE FAIL");
        return -1;
    }
    return fd;
}   

void TimerWheel::HandleAllTask(size_t index)
{
    wheel_[index].clear();
}

void TimerWheel::TimerFdReadCallback()
{
    uint64_t expirations;
    ssize_t ret = read(timerfd_, &expirations, sizeof(expirations));

    for(int i = 0 ; i < expirations ; i++)
    {
        tick_ = tick_ % wheel_.size();
        HandleAllTask(tick_);
    }
}

TimerWheel::TimerWheel(std::shared_ptr<EventLoop> eventloop)
:tick_(0) ,
wheel_(60) ,
timerfd_(CreateTimerFd()) ,
eventloop_(eventloop) , 
channel_(std::make_shared<Channel>(timerfd_ , eventloop))
{
    channel_->SetReadCallback(std::bind(&TimerWheel::TimerFdReadCallback , this));
}

void TimerWheel::SetTask(size_t id ,size_t timeout , Task task)
{
    std::shared_ptr<TimeTask> task_ptr = std::make_shared<TimeTask>(id , timeout , task);
    int index = (tick_ + timeout) % wheel_.size();
    wheel_[index].push_back(task_ptr);
    tasks_.emplace(id , task_ptr);
}

void TimerWheel::UpdateTask(size_t id)
{
    auto it = tasks_.find(id);
    if(it == tasks_.end())
    {
        return ;
    }
    auto task = it->second.lock();
    int timeout = task->Timeout();
    int index = (tick_ + timeout) % wheel_.size();
    wheel_[index].push_back(task);
}

void TimerWheel::CancelTask(size_t id)
{
    auto it = tasks_.find(id);
    if(it == tasks_.end())
    {
        return ;
    }
    auto task = it->second.lock();
    task->Cancel();
}

void TimerWheel::Ready()
{
    struct itimerspec ts;
    ts.it_interval.tv_sec = 1;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = 1;
    ts.it_value.tv_nsec = 0;
    int ret = timerfd_settime(timerfd_, 0, &ts, NULL);
    if (ret) {
        QLOG_ERROR("TIMER SETTIME FAIL");
    }

    channel_->ReadAble();
}