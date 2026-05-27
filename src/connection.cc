
#include "connection.h"
#include "base/channel.h"
#include "server/eventloop.h"
#include "base/util/quill_log.h"


void Connection::ReleaseInLoop()
{
    // 关闭当前连接
    // 1. 关闭事件监控
    // 2. 调用 用户close回调
    // 3. 调用 svr_close 回调 , 移除 conn

    eventloop_->RemvoeEvent(channel_);
    if(close_callback_)
    {
        close_callback_(shared_from_this());
    }
    if(svr_close_callback_)
    {
        svr_close_callback_(shared_from_this());
    }
}


void Connection::ConnReadCallback()
{
    std::string buf;
    ssize_t ret = socket_.RecvNoBlock(buf);
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
        {
            return;
        }
        status_ = ConnectionStatus::DISCONNECTING;
        if (in_buffer_->Size() > 0)
        {
            message_callback_(shared_from_this(), in_buffer_);
            channel_->WriteAble();
        }
        else
        {
            Release();
        }
        return;
    }
    if (ret == 0)
    {
        status_ = ConnectionStatus::DISCONNECTING;
        if (in_buffer_->Size() > 0)
        {
            message_callback_(shared_from_this(), in_buffer_);
            channel_->WriteAble();
        }
        else
        {
            Release();
        }
        return;
    }
    in_buffer_->WriteAndPush(buf);

    if (message_callback_ && in_buffer_->Size() > 0)
    {
        message_callback_(shared_from_this(), in_buffer_);
        channel_->WriteAble();
    }
}
void Connection::ConnWriteCallback()
{
    // 先 peek 数据，再按实际发送量 pop，避免数据丢失
    std::string buf;
    out_buffer_->Read(buf);
    if (buf.empty())
    {
        channel_->UnWriteAble();
        return;
    }

    ssize_t ret = socket_.SendNoBlock(buf);
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
        {
            return;  // 数据留在缓冲区，等待下次可写
        }
        status_ = ConnectionStatus::DISCONNECTING;
        out_buffer_->Clear();
        return;
    }
    out_buffer_->MoveReadAddr(static_cast<size_t>(ret));

    if (status_ == ConnectionStatus::DISCONNECTING)
    {
        Release();
    }
    if (out_buffer_->Size() == 0)
    {
        channel_->UnWriteAble();
    }
}
void Connection::ConnErrorCallback()
{
    Release();
}
void Connection::ConnCloseCallback()
{
    Release();
}
void Connection::ConnEventCallback()
{
    // 属性连接销毁事件
    if(is_selfrelease_)
    {
        eventloop_->UpdateTimedJob(conn_id_);
    }
    if(event_callback_)
    {
        event_callback_(shared_from_this());
    }
}

void Connection::Release()
{
    status_ = ConnectionStatus::DISCONNECTED;
    QLOG_INFO("CONNECT : {} BE RELAEASE" , conn_id_);
    eventloop_->PutIntoQueue(std::bind(&Connection::ReleaseInLoop , this));
}

void Connection::SendMessageInLoop(const std::string& message)
{
    // 发送消息
    out_buffer_->WriteAndPush(message);
    channel_->WriteAble();
}

Connection::Connection(uint64_t conn_id , std::shared_ptr<EventLoop> eventloop , int fd)
:conn_id_(conn_id) , 
eventloop_(eventloop) , 
channel_(std::make_shared<Channel>(fd , eventloop_)) , 
in_buffer_(std::make_shared<Buffer>()) , 
out_buffer_(std::make_shared<Buffer>()) , 
socket_(fd) , 
is_selfrelease_(false) , 
timeout_(0) , 
status_(ConnectionStatus::CONNECTING)
{
    QLOG_INFO("NEW CONNECTION : {} , THE EVENTLOOP IS : {}" , conn_id_, (void*)eventloop_.get());
    channel_->SetReadCallback(std::bind(&Connection::ConnReadCallback , this));
    channel_->SetWriteCallback(std::bind(&Connection::ConnWriteCallback , this));
    channel_->SetCloseCallback(std::bind(&Connection::ConnCloseCallback , this));
    channel_->SetErrorCallback(std::bind(&Connection::ConnErrorCallback , this));
    channel_->SetEventCallback(std::bind(&Connection::ConnEventCallback , this));
}


void Connection::SendMessage(const std::string& message)
{
    eventloop_->RunInLoop(std::bind(&Connection::SendMessageInLoop , this , message));
    return;
}


void Connection::ReadyInLoop()
{
    status_ = ConnectionStatus::CONNECTED;
    if(is_selfrelease_)
    {
        eventloop_->AddTimedJob(conn_id_ , timeout_ , std::bind(&Connection::Release , this));
    }
    channel_->ReadAble();
}

void Connection::Ready()
{
    if(newconnect_callback_)
        newconnect_callback_(shared_from_this());
    eventloop_->RunInLoop(std::bind(&Connection::ReadyInLoop , this));
}
