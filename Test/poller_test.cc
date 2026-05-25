#include <gtest/gtest.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <memory>

#include "base/poller.h"
#include "base/channel.h"

class PollerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        poller_ = std::make_unique<Poller>();
        ASSERT_GT(poller_->Fd(), 0);
    }

    void TearDown() override
    {
        poller_.reset();
    }

    static std::shared_ptr<Channel> MakeEventChannel()
    {
        int fd = eventfd(0, EFD_NONBLOCK);
        EXPECT_GT(fd, 0);
        return std::make_shared<Channel>(fd);
    }

    static void TriggerEvent(Channel *channel)
    {
        uint64_t val = 1;
        EXPECT_EQ(write(channel->Fd(), &val, sizeof(val)), (ssize_t)sizeof(val));
    }

    std::unique_ptr<Poller> poller_;
};

// 构造函数创建有效的 epoll fd
TEST_F(PollerTest, ConstructorCreatesValidEpollFd)
{
    EXPECT_GT(poller_->Fd(), 0);
}

// 第一次 UpdateEvent 走 EPOLL_CTL_ADD 路径，应返回 0
TEST_F(PollerTest, UpdateEventAddsChannel)
{
    auto channel = MakeEventChannel();
    channel->ReadAble();

    int ret = poller_->UpdateEvent(channel);
    EXPECT_EQ(ret, 0);
}

// 对已注册 fd 再次 UpdateEvent 应走 EPOLL_CTL_MOD，返回 0
TEST_F(PollerTest, UpdateEventModifiesExistingChannel)
{
    auto channel = MakeEventChannel();
    channel->ReadAble();

    ASSERT_EQ(poller_->UpdateEvent(channel), 0);

    channel->WriteAble();
    int ret = poller_->UpdateEvent(channel);
    EXPECT_EQ(ret, 0);
}

// 传入 events=0 的 channel 应能正常 ADD
TEST_F(PollerTest, UpdateEventWithZeroEvents)
{
    auto channel = MakeEventChannel();
    // 不设置任何事件，events_ 默认 0

    int ret = poller_->UpdateEvent(channel);
    EXPECT_EQ(ret, 0);
}

// 移除已注册的 channel 应成功返回 0
TEST_F(PollerTest, RemvoeEventRegisteredChannel)
{
    auto channel = MakeEventChannel();
    channel->ReadAble();

    ASSERT_EQ(poller_->UpdateEvent(channel), 0);

    int ret = poller_->RemvoeEvent(channel);
    EXPECT_EQ(ret, 0);
}

// 移除未注册的 channel 应返回 -1
TEST_F(PollerTest, RemvoeEventUnknownChannel)
{
    auto channel = MakeEventChannel();

    int ret = poller_->RemvoeEvent(channel);
    EXPECT_EQ(ret, -1);
}

// epoll_wait 能检测到 EPOLLIN 事件
TEST_F(PollerTest, EpollDetectsReadEvent)
{
    auto channel = MakeEventChannel();
    channel->ReadAble();  // events_ |= EPOLLIN

    ASSERT_EQ(poller_->UpdateEvent(channel), 0);

    TriggerEvent(channel.get());

    struct epoll_event events[8];
    int nfds = epoll_wait(poller_->Fd(), events, 8, 0);
    EXPECT_EQ(nfds, 1);
    EXPECT_EQ(events[0].data.fd, channel->Fd());
    EXPECT_TRUE(events[0].events & EPOLLIN);
}

// eventfd 默认就可写，epoll_wait 能检测到 EPOLLOUT
TEST_F(PollerTest, EpollDetectsWriteEvent)
{
    auto channel = MakeEventChannel();
    channel->WriteAble();  // events_ |= EPOLLOUT

    ASSERT_EQ(poller_->UpdateEvent(channel), 0);

    struct epoll_event events[8];
    int nfds = epoll_wait(poller_->Fd(), events, 8, 0);
    EXPECT_EQ(nfds, 1);
    EXPECT_EQ(events[0].data.fd, channel->Fd());
    EXPECT_TRUE(events[0].events & EPOLLOUT);
}

// 同时注册两个 channel，只触发其中一个
TEST_F(PollerTest, MultipleChannelsIndependent)
{
    auto ch1 = MakeEventChannel();
    auto ch2 = MakeEventChannel();
    ch1->ReadAble();
    ch2->ReadAble();

    ASSERT_EQ(poller_->UpdateEvent(ch1), 0);
    ASSERT_EQ(poller_->UpdateEvent(ch2), 0);

    // 只触发 ch1
    TriggerEvent(ch1.get());

    struct epoll_event events[8];
    int nfds = epoll_wait(poller_->Fd(), events, 8, 0);
    EXPECT_EQ(nfds, 1);
    EXPECT_EQ(events[0].data.fd, ch1->Fd());
    EXPECT_TRUE(events[0].events & EPOLLIN);
}

// channel 同时监听读写事件
TEST_F(PollerTest, ChannelWithBothReadAndWriteEvents)
{
    auto channel = MakeEventChannel();
    channel->ReadAble();
    channel->WriteAble();

    ASSERT_EQ(poller_->UpdateEvent(channel), 0);

    struct epoll_event events[8];
    int nfds = epoll_wait(poller_->Fd(), events, 8, 0);
    EXPECT_EQ(nfds, 1);
    EXPECT_EQ(events[0].data.fd, channel->Fd());
    EXPECT_TRUE(events[0].events & EPOLLOUT);

    TriggerEvent(channel.get());

    nfds = epoll_wait(poller_->Fd(), events, 8, 0);
    EXPECT_EQ(nfds, 1);
    EXPECT_TRUE(events[0].events & EPOLLIN);
}

// 未触发事件时 epoll_wait 超时返回 0
TEST_F(PollerTest, EpollWaitTimeoutWithNoEvents)
{
    auto channel = MakeEventChannel();
    channel->ReadAble();

    ASSERT_EQ(poller_->UpdateEvent(channel), 0);

    // 没有写入 eventfd，epoll_wait 应超时返回 0
    struct epoll_event events[8];
    int nfds = epoll_wait(poller_->Fd(), events, 8, 10);
    EXPECT_EQ(nfds, 0);
}
