
#include "server/eventloop.h"
#include "base/util/quill_log.h"

#include <sys/eventfd.h>


EventLoop::EventLoop()
:id_(std::this_thread::get_id()) , 
poller_() , 
eventfd_(CreateEventFd()) , 
tasks_()
{
}


void EventLoop::Init()
{
    channel_ = std::make_shared<Channel>(eventfd_ , shared_from_this());
    timerwheel_ = std::make_unique<TimerWheel>(shared_from_this());
    channel_->SetReadCallback(std::bind(&EventLoop::EventFdReadCallback , this));
}

EventLoop::~EventLoop()
{
    close(eventfd_);
}

int EventLoop::CreateEventFd()
{
    int fd = eventfd(0 , EFD_NONBLOCK | EFD_SEMAPHORE);
    if(fd < 0)
    {
        QLOG_ERROR("EVENT FD CREATE FAIL");
        return -1;
    }
    return fd;
}

void EventLoop::EventFdReadCallback()
{
    uint64_t u;
    ssize_t ret = read(eventfd_ , &u , sizeof(u));
    if(ret < 0)
    {
        if(errno == EAGAIN || errno == EINTR)
        {
            return;
        }
        QLOG_WARN("EVENTFD READ FAIL: {} " , strerror(errno));
        return ;
    }

    return;
}

void EventLoop::NotifyThreadInLoop()
{
    uint64_t u = 1;
    ssize_t ret = write(eventfd_ , &u , sizeof(u));
    if(ret < 0)
    {
        QLOG_WARN("EVENTFD WRITE FAIL : {} " , strerror(errno));
        return ;
    }
    return ;
}

void EventLoop::NotifyThread()
{
    RunInLoop(std::bind(&EventLoop::NotifyThreadInLoop , this ));
    return ;
}


bool EventLoop::AddTimedJobInLoop(size_t id , size_t timeout , Task task)
{
    timerwheel_->SetTask(id , timeout , task);
    return true;
}
bool EventLoop::UpdateTimedJobInLoop(size_t id)
{
    timerwheel_->UpdateTask(id);
    return true;
}
bool EventLoop::CancelTimedJobInLoop(size_t id)
{
    timerwheel_->CancelTask(id);
    return true;
}

bool EventLoop::AddTimedJob(size_t id , size_t timeout , Task task)
{
    RunInLoop(std::bind(&EventLoop::AddTimedJobInLoop , this , id , timeout , task));
    return true;
}
bool EventLoop::UpdateTimedJob(size_t id)
{
    RunInLoop(std::bind(&EventLoop::UpdateTimedJobInLoop , this , id ));
    return true;
}
bool EventLoop::CancelTimedJob(size_t id)
{
    RunInLoop(std::bind(&EventLoop::CancelTimedJobInLoop , this , id));
    return true;
}

bool EventLoop::IsInLoop()
{
    if(id_ == std::this_thread::get_id())
    {
        return true;
    }
    return false;
}

void EventLoop::RunInLoop(Task task)
{
    if(IsInLoop())
    {
        task();
        return ;
    }
    PutIntoQueue(task);
    return ;
}

void EventLoop::PutIntoQueue(Task task)
{
    tasks_.push_back(task);
}


void EventLoop::UpdateEventInLoop(std::shared_ptr<Channel> channel)
{
    poller_.UpdateEvent(channel);
    return ;
}
void EventLoop::RemvoeEventInLoop(std::shared_ptr<Channel> channel)
{
    poller_.RemvoeEvent(channel);
    return ;
}

void EventLoop::UpdateEvent(std::shared_ptr<Channel> channel)
{
    RunInLoop(std::bind(&EventLoop::UpdateEventInLoop , this , channel ));
    return ;
}
void EventLoop::RemvoeEvent(std::shared_ptr<Channel> channel)
{
    RunInLoop(std::bind(&EventLoop::RemvoeEventInLoop , this , channel));
    return ;    
}

void EventLoop::HanleTask()
{
    std::vector<std::shared_ptr<Channel> > ready_event = poller_.EpollWait();
    for(auto& channel : ready_event)
    {
        channel->Handle();
        channel->SetRevents(0);
    }
    for(auto &task : tasks_)
    {
        task();
    }
    tasks_.clear();
}