
#pragma once

/*
    - 对 Poll 进行封装
    - 成员 : 
            1) pollfd 
            2) 哈希表 : 关系所有的 Channel
    - 接口 : 
            1) 创建 pollfd
            2) 更新监听事件
            3) 移除监听

*/

#include <unistd.h>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>

class Channel;
class PollerTest;

class Poller
{
private:
    int epollfd_;
    std::unordered_map<int , std::shared_ptr<Channel> > channels_;

private:
    int CreatePoll();

    int EpollControl(int op , int fd, int event);

public:
    Poller()
    {
        epollfd_ = CreatePoll();
    }

    ~Poller()
    {
        close(epollfd_);
    }

    int Fd()
    {
        return epollfd_;
    }

    int UpdateEvent(std::shared_ptr<Channel> channel);
    int RemvoeEvent(std::shared_ptr<Channel> channel);

    std::vector<std::shared_ptr<Channel> > EpollWait();  // 返回所有就绪事件
};
