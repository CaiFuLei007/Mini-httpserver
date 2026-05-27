

#include "tcp_server.h"

void TcpServer::CloseConnection(std::shared_ptr<Connection> conn)
{
    std::unique_lock lock(mutex_);
    int id = conn->Id();
    connections_.erase(id);
}

void TcpServer::AcceptCallback(int fd)
{
    // 创建新连接
    std::shared_ptr<EventLoop> next_loop;
    if(threadpoll_.Size() == 0)
    {
        next_loop = eventloop_;
    }
    else
    {
        next_loop = threadpoll_.NextEventLoop();
    }
    auto new_conn = std::make_shared<Connection>(next_id_ , next_loop , fd);
    new_conn->SetMessageCallback(message_callback_);
    new_conn->SetNewConnectCallback(newconnect_callback_);
    new_conn->SetEventCallback(event_callback_);
    new_conn->SetCloseCallback(close_callback_);
    new_conn->SetSvrCloseCallback(std::bind(&TcpServer::CloseConnection , this , std::placeholders::_1));

    if(is_selfrelease_)
    {
        new_conn->SetSelfRelease(timeout_);
    }
    
    std::unique_lock lock(mutex_);
    connections_.emplace(next_id_++ , new_conn);
    new_conn->Ready();
    return ;
}

TcpServer::TcpServer(uint16_t port)
:eventloop_(std::make_shared<EventLoop>()) , 
acceptor_(port , eventloop_) ,
threadpoll_() , 
connections_() , 
next_id_(0)  , 
is_selfrelease_(false) , 
timeout_(0)
{
    eventloop_->Init();
    acceptor_.SetNewConnectCallback(std::bind(&TcpServer::AcceptCallback , this , std::placeholders::_1));
}

void TcpServer::SetScheduledTasks(size_t timeout , Task task)
{
    eventloop_->AddTimedJob(next_id_++ , timeout , task);
}
void TcpServer::UpdateScheduledTasks(size_t id)
{
    eventloop_->UpdateTimedJob(id);
}
void TcpServer::RemoveScheduledTasks(size_t id)
{
    eventloop_->CancelTimedJob(id);
}

void TcpServer::Run()
{
    threadpoll_.Run();
    acceptor_.Listen();
    eventloop_->HanleTask();
}