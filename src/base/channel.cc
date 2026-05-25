  
#include "base/channel.h"

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

void Channel::ReadAble()
{
    events_ |= EPOLLIN;
}

void Channel::WriteAble()
{
    events_ |= EPOLLOUT;
}
void Channel::UnReadAble()
{
    events_ &= (~EPOLLIN);
}
void Channel::UnWriteAble()
{
    events_ &= (~EPOLLOUT);
}