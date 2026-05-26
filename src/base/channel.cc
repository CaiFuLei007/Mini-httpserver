  
#include "base/channel.h"
#include "tcp_server/eventloop.h"

#include <sys/epoll.h>
  
void Channel::Handle()
{
    if(events_ & EPOLLIN)
    {
        if(read_callback_)
            read_callback_();
    }

    if(events_ & EPOLLOUT)
    {
        if(write_callback_)
            write_callback_();
    }

    if(events_ & EPOLLHUP || events_ & EPOLLERR)
    {
        if(error_callback_)
            error_callback_();
    }

    if(event_callback_)
    {
        event_callback_();
    }
}

void Channel::Update()
{
    eventloop_.lock()->UpdateEvent(shared_from_this());
}
void Channel::Remove()
{
    eventloop_.lock()->RemvoeEvent(shared_from_this());
}

void Channel::ReadAble()
{
    events_ |= EPOLLIN;
    Update();
}

void Channel::WriteAble()
{
    events_ |= EPOLLOUT;
    Update();
}

void Channel::UnReadAble()
{
    events_ &= (~EPOLLIN);
    Update();
}

void Channel::UnWriteAble()
{
    events_ &= (~EPOLLOUT);
    Update();
}

void Channel::RemoveAllEvent()
{
    events_ = revents_ = 0;
    Remove();
}