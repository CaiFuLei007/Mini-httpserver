
#include "server/acceptor.h"
#include "server/eventloop.h"
#include "base/channel.h"
#include "base/util/quill_log.h"


void Acceptor::AcceptReadCallback()
{
    int fd = socket_.Accept();
    if(fd < 0)
    {
        QLOG_ERROR("SOCKET ACCEPT FAIL");
        return ;
    }

    if(new_connect_cb_)
    {
        new_connect_cb_(fd);
        return ;
    }
}


Acceptor::Acceptor(uint16_t port, std::shared_ptr<EventLoop> eventloop)
:socket_() , 
channel_() ,
eventloop_(eventloop)
{
    socket_.CreateServer(port);
    channel_ = std::make_shared<Channel>(socket_.Fd() , eventloop_.lock());
    channel_->SetReadCallback(std::bind(&Acceptor::AcceptReadCallback , this));
}

void Acceptor::Listen()
{
    channel_->ReadAble();
}