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

static std::shared_ptr<EventLoop> MakeConnLoop()
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
    auto loop = MakeConnLoop();
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    close(fds[1]);

    EXPECT_NO_THROW(std::make_shared<Connection>(1, loop, fds[0]));
}

TEST(ConnectionTest, Construct_WithDifferentIds)
{
    auto loop = MakeConnLoop();

    auto conn1 = MakeConnection(1, loop);
    auto conn2 = MakeConnection(2, loop);
    auto conn3 = MakeConnection(9999, loop);

    EXPECT_NE(conn1, conn2);
    EXPECT_NE(conn2, conn3);
}

TEST(ConnectionTest, Construct_MultipleConnections_NoSharedState)
{
    auto loop = MakeConnLoop();

    auto conn1 = MakeConnection(100, loop);
    auto conn2 = MakeConnection(200, loop);

    EXPECT_NO_THROW(conn1->SetSelfRelease(10));
    EXPECT_NO_THROW(conn2->SetSelfRelease(20));
}

// ============================================================
// SetMessageCallback
// ============================================================

TEST(ConnectionTest, SetMessageCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    bool called = false;

    EXPECT_NO_THROW(conn->SetMessageCallback([&called](auto, auto) { called = true; }));
}

TEST(ConnectionTest, SetMessageCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetMessageCallback(nullptr));
}

TEST(ConnectionTest, SetMessageCallback_Overwrite)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    int value = 0;

    conn->SetMessageCallback([&value](auto, auto) { value = 1; });
    conn->SetMessageCallback([&value](auto, auto) { value = 2; });

    EXPECT_EQ(value, 0);
}

// ============================================================
// SetNewConnectCallback
// ============================================================

TEST(ConnectionTest, SetNewConnectCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    EXPECT_NO_THROW(conn->SetNewConnectCallback([](auto) {}));
}

TEST(ConnectionTest, SetNewConnectCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetNewConnectCallback(nullptr));
}

// ============================================================
// SetEventCallback
// ============================================================

TEST(ConnectionTest, SetEventCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    EXPECT_NO_THROW(conn->SetEventCallback([](auto) {}));
}

TEST(ConnectionTest, SetEventCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetEventCallback(nullptr));
}

// ============================================================
// SetCloseCallback
// ============================================================

TEST(ConnectionTest, SetCloseCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    EXPECT_NO_THROW(conn->SetCloseCallback([](auto) {}));
}

TEST(ConnectionTest, SetCloseCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetCloseCallback(nullptr));
}

// ============================================================
// SetSvrCloseCallback
// ============================================================

TEST(ConnectionTest, SetSvrCloseCallback_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    EXPECT_NO_THROW(conn->SetSvrCloseCallback([](auto) {}));
}

TEST(ConnectionTest, SetSvrCloseCallback_Nullptr_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetSvrCloseCallback(nullptr));
}

// ============================================================
// All callbacks together
// ============================================================

TEST(ConnectionTest, SetAllCallbacks_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    int count = 0;

    EXPECT_NO_THROW({
        conn->SetMessageCallback([&count](auto, auto) { ++count; });
        conn->SetNewConnectCallback([&count](auto) { ++count; });
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
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SendMessage("hello"));
}

TEST(ConnectionTest, SendMessage_EmptyString_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SendMessage(""));
}

TEST(ConnectionTest, SendMessage_LargeString_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    std::string large(65536, 'X');
    EXPECT_NO_THROW(conn->SendMessage(large));
}

TEST(ConnectionTest, SendMessage_Multiple_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    for (int i = 0; i < 100; ++i)
        EXPECT_NO_THROW(conn->SendMessage("msg"));
}

// ============================================================
// Release
// ============================================================

TEST(ConnectionTest, Release_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->Release());
}

TEST(ConnectionTest, Release_Multiple_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    conn->Release();
    conn->Release();
    conn->Release();

    SUCCEED();
}

TEST(ConnectionTest, SendMessage_AfterRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    conn->Release();
    EXPECT_NO_THROW(conn->SendMessage("data"));
}

// ============================================================
// SetSelfRelease
// ============================================================

TEST(ConnectionTest, SetSelfRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetSelfRelease(30));
}

TEST(ConnectionTest, SetSelfRelease_ZeroTimeout_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetSelfRelease(0));
}

TEST(ConnectionTest, SetSelfRelease_LargeTimeout_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->SetSelfRelease(65535));
}

// ============================================================
// Ready
// ============================================================

TEST(ConnectionTest, Ready_WithoutSelfRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());
    EXPECT_NO_THROW(conn->Ready());
}

TEST(ConnectionTest, Ready_WithSelfRelease_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    conn->SetSelfRelease(30);
    EXPECT_NO_THROW(conn->Ready());
}

TEST(ConnectionTest, Ready_Multiple_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    conn->Ready();
    conn->Ready();
    conn->Ready();

    SUCCEED();
}

TEST(ConnectionTest, Ready_AfterSendMessage_NoThrow)
{
    auto conn = MakeConnection(1, MakeConnLoop());

    conn->SendMessage("hello");
    EXPECT_NO_THROW(conn->Ready());
}

// ============================================================
// Integration tests removed — HanleTask() now runs infinite loop,
// cannot be used for synchronous unit test validation.
// These scenarios should be tested via end-to-end integration tests
// (e.g. wrk benchmarks or standalone echo server tests).
// ============================================================

// ============================================================
// Thread safety
// ============================================================

TEST(ConnectionTest, SendMessage_FromOtherThread_NoThrow)
{
    auto loop = MakeConnLoop();
    auto conn = MakeConnection(1, loop);

    std::thread t([&conn]() {
        EXPECT_NO_THROW(conn->SendMessage("from thread"));
    });
    t.join();
}

TEST(ConnectionTest, Release_FromOtherThread_NoThrow)
{
    auto loop = MakeConnLoop();
    auto conn = MakeConnection(1, loop);

    std::thread t([&conn]() {
        EXPECT_NO_THROW(conn->Release());
    });
    t.join();
}

TEST(ConnectionTest, SetCallbacks_FromOtherThread_NoThrow)
{
    auto loop = MakeConnLoop();
    auto conn = MakeConnection(1, loop);

    std::thread t([&conn]() {
        conn->SetMessageCallback([](auto, auto) {});
        conn->SetNewConnectCallback([](auto) {});
        conn->SetEventCallback([](auto) {});
        conn->SetCloseCallback([](auto) {});
        conn->SetSvrCloseCallback([](auto) {});
    });
    t.join();

    SUCCEED();
}

TEST(ConnectionTest, Concurrent_SendAndRelease)
{
    auto loop = MakeConnLoop();
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
    auto loop = MakeConnLoop();

    {
        auto conn = MakeConnection(1, loop);
        conn->SetMessageCallback([](auto, auto) {});
        conn->SendMessage("data");
        conn->SetSelfRelease(30);
    }

    SUCCEED();
}

TEST(ConnectionTest, Destroy_AfterReady_NoCrash)
{
    auto loop = MakeConnLoop();

    {
        auto conn = MakeConnection(1, loop);
        conn->Ready();
        conn->SendMessage("data");
    }

    SUCCEED();
}

TEST(ConnectionTest, FullLifecycle_SendReadyRelease)
{
    auto loop = MakeConnLoop();

    auto conn = MakeConnection(1, loop);
    conn->SetMessageCallback([](auto, auto) {});
    conn->SetNewConnectCallback([](auto) {});
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
    auto loop = MakeConnLoop();

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
    auto loop = MakeConnLoop();
    auto conn = MakeConnection(1, loop);

    conn->SetSelfRelease(60);
    conn->Ready();
    conn->Release();

    SUCCEED();
}

TEST(ConnectionTest, ZeroTimeout_Ready_WontCrash)
{
    auto loop = MakeConnLoop();
    auto conn = MakeConnection(1, loop);

    conn->SetSelfRelease(0);
    conn->Ready();

    // Timed job with 0 timeout is scheduled but doesn't fire yet (not processed)
    SUCCEED();
}
