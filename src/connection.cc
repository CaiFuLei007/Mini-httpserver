
#include "connection.h"
#include "base/channel.h"
#include "server/eventloop.h"


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
    // 连接读取事件的回调
    // 1. 将数据读取到缓冲区中
    // 2. 调用用户的回调
    // 3. 打开写事件

    std::string buf;
    int ret = socket_.RecvNoBlock(buf);
    if(ret < 0)
    {
        if(errno == EAGAIN || errno == EINTR)
        {
            return ;
        }
        // 出现了错误
        // 读取缓冲区中如果有数据就进行处理 , 然后发送 , 最后关闭连接
        if(in_buffer_->Size() > 0)
        {
            read_callback_(shared_from_this() , in_buffer_);
            channel_->WriteAble();
        }
        return ;
    }
    in_buffer_->WriteAndPush(buf);

    if(read_callback_)
    {
        read_callback_(shared_from_this() , in_buffer_);
        channel_->WriteAble();
        return ;
    }
}
void Connection::ConnWriteCallback()
{
    // 连接写事件的回调
    // 1. 将数据从缓冲区中发送出去
    std::string buf;
    out_buffer_->ReadAndPop(buf);
    int ret = socket_.Send(buf);
    if(ret < 0)
    {
        if(errno == EAGAIN || errno == EINTR)
        {
            return ;
        }
        // 将发送缓冲区清空 , 调用 release 销毁连接
        out_buffer_->Clear();
        Release();
        return ;
    }
    if(out_buffer_->Size() == 0)
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
}

void Connection::Release()
{
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
timeout_(0)
{
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

void Connection::Ready()
{
    if(is_selfrelease_)
    {
        eventloop_->AddTimedJob(conn_id_ , timeout_ , std::bind(&Connection::Release , this));
    }
    channel_->ReadAble();
}