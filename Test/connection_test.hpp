#pragma once

#include <gtest/gtest.h>
#include "connection.h"
#include "server/eventloop.h"

#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <thread>

// ============================================================
// Helpers
// ============================================================

static std::shared_ptr<EventLoop> MakeLoop()
{
    auto loop = std::make_shared<EventLoop>();
    loop->Init();
    return loop;
}

static std::shared_ptr<Connection> MakeConnection(uint64_t conn_id, std::shared_ptr<EventLoop> loop)
{
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    close(fds[1]);
    return std::make_shared<Connection>(conn_id, loop, fds[0]);
}

// ============================================================
// Construction
// ============================================================

TEST(ConnectionTest, Construct_NoThrow)
{
    auto loop = MakeLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    close(fds[1]);

    EXPECT_NO_THROW(std::make_shared<Connection>(1, loop, fds[0]));
}

TEST(ConnectionTest, Construct_WithDifferentIds)
{
    auto loop = MakeLoop();

    auto conn1 = MakeConnection(1, loop);
    auto conn2 = MakeConnection(2, loop);
    auto conn3 = MakeConnection(9999, loop);

    EXPECT_NE(conn1, conn2);
    EXPECT_NE(conn2, conn3);
}

TEST(ConnectionTest, Construct_MultipleConnections_NoSharedState)
{
    auto loop = MakeLoop();

    auto conn1 = MakeConnection(100, loop);
    auto conn2 = MakeConnection(200, loop);

    EXPECT_NO_THROW(conn1->SetSelfRelease(10));
    EXPECT_NO_THROW(conn2->SetSelfRelease(20));
}

// ============================================================
// SetReadCallback
// ============================================================

TEST(ConnectionTest, SetReadCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    bool called = false;

    EXPECT_NO_THROW(conn->SetReadCallback([&called](auto, auto) { called = true; }));
}

TEST(ConnectionTest, SetReadCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetReadCallback(nullptr));
}

TEST(ConnectionTest, SetReadCallback_Overwrite)
{
    auto conn = MakeConnection(1, MakeLoop());
    int value = 0;

    conn->SetReadCallback([&value](auto, auto) { value = 1; });
    conn->SetReadCallback([&value](auto, auto) { value = 2; });

    EXPECT_EQ(value, 0);
}

// ============================================================
// SetWriteCallback
// ============================================================

TEST(ConnectionTest, SetWriteCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    EXPECT_NO_THROW(conn->SetWriteCallback([](auto) {}));
}

TEST(ConnectionTest, SetWriteCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetWriteCallback(nullptr));
}

// ============================================================
// SetErrorCallback
// ============================================================

TEST(ConnectionTest, SetErrorCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    EXPECT_NO_THROW(conn->SetErrorCallback([](auto) {}));
}

TEST(ConnectionTest, SetErrorCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetErrorCallback(nullptr));
}

// ============================================================
// SetEventCallback
// ============================================================

TEST(ConnectionTest, SetEventCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    EXPECT_NO_THROW(conn->SetEventCallback([](auto) {}));
}

TEST(ConnectionTest, SetEventCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetEventCallback(nullptr));
}

// ============================================================
// SetCloseCallback
// ============================================================

TEST(ConnectionTest, SetCloseCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    EXPECT_NO_THROW(conn->SetCloseCallback([](auto) {}));
}

TEST(ConnectionTest, SetCloseCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetCloseCallback(nullptr));
}

// ============================================================
// SetSvrCloseCallback
// ============================================================

TEST(ConnectionTest, SetSvrCloseCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    EXPECT_NO_THROW(conn->SetSvrCloseCallback([](auto) {}));
}

TEST(ConnectionTest, SetSvrCloseCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetSvrCloseCallback(nullptr));
}

// ============================================================
// All callbacks together
// ============================================================

TEST(ConnectionTest, SetAllCallbacks_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    int count = 0;

    EXPECT_NO_THROW({
        conn->SetReadCallback([&count](auto, auto) { ++count; });
        conn->SetWriteCallback([&count](auto) { ++count; });
        conn->SetErrorCallback([&count](auto) { ++count; });
        conn->SetEventCallback([&count](auto) { ++count; });
        conn->SetCloseCallback([&count](auto) { ++count; });
        conn->SetSvrCloseCallback([&count](auto) { ++count; });
    });
}

// ============================================================
// SendMessage — same thread (executes immediately via RunInLoop)
// ============================================================

TEST(ConnectionTest, SendMessage_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SendMessage("hello"));
}

TEST(ConnectionTest, SendMessage_EmptyString_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SendMessage(""));
}

TEST(ConnectionTest, SendMessage_LargeString_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    std::string large(65536, 'X');
    EXPECT_NO_THROW(conn->SendMessage(large));
}

TEST(ConnectionTest, SendMessage_Multiple_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    for (int i = 0; i < 100; ++i)
        EXPECT_NO_THROW(conn->SendMessage("msg"));
}

// ============================================================
// Release
// ============================================================

TEST(ConnectionTest, Release_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->Release());
}

TEST(ConnectionTest, Release_Multiple_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    conn->Release();
    conn->Release();
    conn->Release();

    SUCCEED();
}

TEST(ConnectionTest, SendMessage_AfterRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    conn->Release();
    EXPECT_NO_THROW(conn->SendMessage("data"));
}

// ============================================================
// SetSelfRelease
// ============================================================

TEST(ConnectionTest, SetSelfRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetSelfRelease(30));
}

TEST(ConnectionTest, SetSelfRelease_ZeroTimeout_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetSelfRelease(0));
}

TEST(ConnectionTest, SetSelfRelease_LargeTimeout_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->SetSelfRelease(65535));
}

// ============================================================
// Ready
// ============================================================

TEST(ConnectionTest, Ready_WithoutSelfRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());
    EXPECT_NO_THROW(conn->Ready());
}

TEST(ConnectionTest, Ready_WithSelfRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    conn->SetSelfRelease(30);
    EXPECT_NO_THROW(conn->Ready());
}

TEST(ConnectionTest, Ready_Multiple_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    conn->Ready();
    conn->Ready();
    conn->Ready();

    SUCCEED();
}

TEST(ConnectionTest, Ready_AfterSendMessage_NoThrow)
{
    auto conn = MakeConnection(1, MakeLoop());

    conn->SendMessage("hello");
    EXPECT_NO_THROW(conn->Ready());
}

// ============================================================
// Integration: data arrives at peer after Send + HandleTask
// ============================================================

TEST(ConnectionTest, Write_SendsDataToPeer)
{
    auto loop = MakeLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    auto conn = std::make_shared<Connection>(1, loop, fds[0]);

    // SendMessage -> out_buffer_ -> WriteAble (EPOLLOUT)
    conn->SendMessage("hello world");

    // Process epoll events: ConnWriteCallback sends data via socket_.Send()
    loop->HanleTask();

    // Read from the peer end of the socketpair
    char buf[256] = {0};
    ssize_t n = read(fds[1], buf, sizeof(buf) - 1);
    EXPECT_EQ(n, 11);
    EXPECT_STREQ(buf, "hello world");

    close(fds[1]);
}

TEST(ConnectionTest, Write_SendsMultipleMessages)
{
    auto loop = MakeLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    auto conn = std::make_shared<Connection>(1, loop, fds[0]);

    conn->SendMessage("msg1");
    loop->HanleTask();

    conn->SendMessage("msg2");
    loop->HanleTask();

    conn->SendMessage("msg3");
    loop->HanleTask();

    char buf[256] = {0};
    ssize_t n = read(fds[1], buf, sizeof(buf) - 1);
    EXPECT_EQ(n, 12);
    EXPECT_STREQ(buf, "msg1msg2msg3");

    close(fds[1]);
}

// ============================================================
// Integration: read callback invoked when peer writes data
// ============================================================

TEST(ConnectionTest, Read_CallbackInvokedOnData)
{
    auto loop = MakeLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    auto conn = std::make_shared<Connection>(1, loop, fds[0]);

    std::atomic<bool> read_called{false};
    std::string received;

    conn->SetReadCallback([&](auto, std::shared_ptr<Buffer> buf) {
        read_called = true;
        buf->ReadAndPop(received);
    });

    // Ready enables EPOLLIN monitoring
    conn->Ready();

    // Write data from the peer end
    const char *msg = "hello from peer";
    ssize_t nw = write(fds[1], msg, strlen(msg));
    ASSERT_GT(nw, 0);

    // Process epoll events: ConnReadCallback reads via RecvNoBlock
    loop->HanleTask();

    EXPECT_TRUE(read_called);
    EXPECT_EQ(received, "hello from peer");

    close(fds[1]);
}

TEST(ConnectionTest, Read_CallbackNotInvokedUntilDataArrives)
{
    auto loop = MakeLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    auto conn = std::make_shared<Connection>(1, loop, fds[0]);

    std::atomic<bool> read_called{false};
    conn->SetReadCallback([&](auto, auto) { read_called = true; });
    conn->Ready();

    // No data written yet — callback should NOT have been called
    EXPECT_FALSE(read_called);

    close(fds[1]);
}

TEST(ConnectionTest, Read_MultipleReads)
{
    auto loop = MakeLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    auto conn = std::make_shared<Connection>(1, loop, fds[0]);

    std::atomic<int> read_count{0};
    std::string all_received;

    conn->SetReadCallback([&](auto, std::shared_ptr<Buffer> buf) {
        read_count++;
        std::string part;
        buf->ReadAndPop(part);
        all_received += part;
    });
    conn->Ready();

    ssize_t nw1 = write(fds[1], "AAA", 3);
    ASSERT_GT(nw1, 0);
    loop->HanleTask();

    ssize_t nw2 = write(fds[1], "BBB", 3);
    ASSERT_GT(nw2, 0);
    loop->HanleTask();

    ssize_t nw3 = write(fds[1], "CCC", 3);
    ASSERT_GT(nw3, 0);
    loop->HanleTask();

    EXPECT_EQ(read_count, 3);
    EXPECT_EQ(all_received, "AAABBBCCC");

    close(fds[1]);
}

// ============================================================
// Integration: callbacks survive after operations
// ============================================================

TEST(ConnectionTest, ReadWrite_RoundTrip)
{
    auto loop = MakeLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    auto conn = std::make_shared<Connection>(1, loop, fds[0]);

    // Set up read callback to capture incoming data
    std::string received;
    conn->SetReadCallback([&](auto, std::shared_ptr<Buffer> buf) {
        buf->ReadAndPop(received);
    });
    conn->Ready();

    // Peer writes to us
    ssize_t nw = write(fds[1], "ping", 4);
    ASSERT_GT(nw, 0);
    loop->HanleTask();
    EXPECT_EQ(received, "ping");

    // We write back to peer
    conn->SendMessage("pong");
    loop->HanleTask();

    char buf[256] = {0};
    ssize_t n = read(fds[1], buf, sizeof(buf) - 1);
    EXPECT_EQ(n, 4);
    EXPECT_STREQ(buf, "pong");

    close(fds[1]);
}

// ============================================================
// Thread safety
// ============================================================

TEST(ConnectionTest, SendMessage_FromOtherThread_NoThrow)
{
    auto loop = MakeLoop();
    auto conn = MakeConnection(1, loop);

    std::thread t([&conn]() {
        EXPECT_NO_THROW(conn->SendMessage("from thread"));
    });
    t.join();
}

TEST(ConnectionTest, Release_FromOtherThread_NoThrow)
{
    auto loop = MakeLoop();
    auto conn = MakeConnection(1, loop);

    std::thread t([&conn]() {
        EXPECT_NO_THROW(conn->Release());
    });
    t.join();
}

TEST(ConnectionTest, SetCallbacks_FromOtherThread_NoThrow)
{
    auto loop = MakeLoop();
    auto conn = MakeConnection(1, loop);

    std::thread t([&conn]() {
        conn->SetReadCallback([](auto, auto) {});
        conn->SetWriteCallback([](auto) {});
        conn->SetErrorCallback([](auto) {});
        conn->SetEventCallback([](auto) {});
        conn->SetCloseCallback([](auto) {});
        conn->SetSvrCloseCallback([](auto) {});
    });
    t.join();

    SUCCEED();
}

TEST(ConnectionTest, Concurrent_SendAndRelease)
{
    auto loop = MakeLoop();
    auto conn = MakeConnection(1, loop);

    std::atomic<int> ok{0};

    auto sender = [&]() {
        for (int i = 0; i < 50; ++i)
        {
            conn->SendMessage("data");
            ok.fetch_add(1);
        }
    };

    auto releaser = [&]() {
        for (int i = 0; i < 50; ++i)
        {
            conn->Release();
            ok.fetch_add(1);
        }
    };

    std::thread t1(sender);
    std::thread t2(releaser);
    t1.join();
    t2.join();

    EXPECT_EQ(ok, 100);
}

// ============================================================
// Edge cases
// ============================================================

TEST(ConnectionTest, Destroy_WithoutReady_NoCrash)
{
    auto loop = MakeLoop();

    {
        auto conn = MakeConnection(1, loop);
        conn->SetReadCallback([](auto, auto) {});
        conn->SendMessage("data");
        conn->SetSelfRelease(30);
    }

    SUCCEED();
}

TEST(ConnectionTest, Destroy_AfterReady_NoCrash)
{
    auto loop = MakeLoop();

    {
        auto conn = MakeConnection(1, loop);
        conn->Ready();
        conn->SendMessage("data");
    }

    SUCCEED();
}

TEST(ConnectionTest, FullLifecycle_SendReadyRelease)
{
    auto loop = MakeLoop();

    auto conn = MakeConnection(1, loop);
    conn->SetReadCallback([](auto, auto) {});
    conn->SetWriteCallback([](auto) {});
    conn->SetCloseCallback([](auto) {});
    conn->SetSvrCloseCallback([](auto) {});

    conn->SendMessage("start");
    conn->Ready();
    conn->SendMessage("more");
    conn->Release();

    SUCCEED();
}

TEST(ConnectionTest, MultipleConnections_SameEventLoop)
{
    auto loop = MakeLoop();

    auto conn1 = MakeConnection(1, loop);
    auto conn2 = MakeConnection(2, loop);
    auto conn3 = MakeConnection(3, loop);

    conn1->SetSelfRelease(10);
    conn2->SetSelfRelease(20);
    conn3->SetSelfRelease(30);

    conn1->Ready();
    conn2->Ready();
    conn3->Ready();

    conn1->SendMessage("a");
    conn2->SendMessage("b");
    conn3->SendMessage("c");

    SUCCEED();
}

TEST(ConnectionTest, SetSelfRelease_ThenReady_ThenRelease)
{
    auto loop = MakeLoop();
    auto conn = MakeConnection(1, loop);

    conn->SetSelfRelease(60);
    conn->Ready();
    conn->Release();

    SUCCEED();
}

TEST(ConnectionTest, ZeroTimeout_Ready_WontCrash)
{
    auto loop = MakeLoop();
    auto conn = MakeConnection(1, loop);

    conn->SetSelfRelease(0);
    conn->Ready();

    // Timed job with 0 timeout is scheduled but doesn't fire yet (not processed)
    SUCCEED();
}
