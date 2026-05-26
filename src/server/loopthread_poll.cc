
#include "server/loopthread_poll.h"
#include "base/util/quill_log.h"



std::shared_ptr<EventLoop> LoopThread::GetEventLoop()
{   
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock , [&](){ return eventloop_ ;});
    return eventloop_;
}


void LoopThread::ThreadCallback()
{
    eventloop_ = std::make_shared<EventLoop>();
    eventloop_->Init();
    cond_.notify_all();
    eventloop_->HanleTask();
}

void LoopThread::Run()
{
    thread_ = std::thread(std::bind(&LoopThread::ThreadCallback , this));
}

// ========================


std::shared_ptr<EventLoop> LoopThreadPoll::NextEventLoop()
{
    next_num_ %= eventloops_.size();
    return eventloops_[next_num_++];
}
void LoopThreadPoll::Run()
{
    loops_.resize(count_);
    eventloops_.resize(count_);
    for(int i = 0 ; i < count_ ; i++)
    {
        loops_[i] = std::make_shared<LoopThread>();
        loops_[i]->Run();

        eventloops_[i] = loops_[i]->GetEventLoop();
    }
}