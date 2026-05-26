
#include "base/poller.h"
#include "base/channel.h"

#include <sys/epoll.h>

int Poller::CreatePoll()
{
    int fd = epoll_create(1);
    return fd;
}

int Poller::EpollControl(int op , int fd , int events)
{
    struct epoll_event epevent;
    epevent.data.fd = fd;
    epevent.events = events;
    return epoll_ctl(epollfd_ , op , fd , &epevent);
}

int Poller::UpdateEvent(std::shared_ptr<Channel> channel)
{
    int fd = channel->Fd();
    auto it = channels_.find(fd);
    if(it == channels_.end())
    {
        channels_.emplace(fd , channel);
        return EpollControl(EPOLL_CTL_ADD , fd , channel->GetEvents());
    }
    return EpollControl(EPOLL_CTL_MOD , fd , channel->GetEvents());
}

int Poller::RemvoeEvent(std::shared_ptr<Channel> channel)
{
    int fd = channel->Fd();
    auto it = channels_.find(fd);
    if(it == channels_.end())
    {
        return -1;
}
    channels_.erase(fd);
    return epoll_ctl(epollfd_ , EPOLL_CTL_DEL , fd , nullptr);
}


std::vector<std::shared_ptr<Channel> > Poller::EpollWait()
{
    struct epoll_event events[1024];
    int nfds = epoll_wait(epollfd_, events, 1024, -1);

    std::vector<std::shared_ptr<Channel> > ret(nfds);
    for (int i = 0; i < nfds; ++i) {
        ret[i] = channels_[events[i].data.fd];
        ret[i]->SetRevents(events[i].events);
    }
    return ret;
}