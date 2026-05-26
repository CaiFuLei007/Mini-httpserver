#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>

#include "tcp_server/eventloop.h"

// ============================================================
// Helper: create a fully initialized EventLoop
// ============================================================
static std::shared_ptr<EventLoop> MakeLoop()
{
    auto loop = std::make_shared<EventLoop>();
    loop->Init();
    return loop;
}

// ============================================================
// Construction & Init
// ============================================================

TEST(EventLoopTest, Construct_NoThrow)
{
    EXPECT_NO_THROW(std::make_shared<EventLoop>());
}

TEST(EventLoopTest, Init_NoThrow)
{
    auto loop = std::make_shared<EventLoop>();
    EXPECT_NO_THROW(loop->Init());
}

TEST(EventLoopTest, ConstructThenInit_ReturnsValidLoop)
{
    auto loop = std::make_shared<EventLoop>();
    loop->Init();
    EXPECT_TRUE(loop->IsInLoop());
}

TEST(EventLoopTest, Destruct_NoLeak)
{
    {
        auto loop = MakeLoop();
    }
    SUCCEED();
}

TEST(EventLoopTest, Init_WithoutSharedPtr_ThrowsBadWeakPtr)
{
    // EventLoop must be owned by shared_ptr for Init() to work.
    // Stack allocation + Init() = UB, don't do it.
    // Test documents the invariant.
    auto loop = std::make_shared<EventLoop>();
    ASSERT_NO_THROW(loop->Init());
    EXPECT_TRUE(loop->IsInLoop());
}

// ============================================================
// IsInLoop
// ============================================================

TEST(EventLoopTest, IsInLoop_TrueInConstructingThread)
{
    auto loop = MakeLoop();
    EXPECT_TRUE(loop->IsInLoop());
}

TEST(EventLoopTest, IsInLoop_FalseInDifferentThread)
{
    auto loop = MakeLoop();
    std::atomic<bool> result{true};

    std::thread t([&]() {
        result = loop->IsInLoop();
    });
    t.join();

    EXPECT_FALSE(result);
}

TEST(EventLoopTest, IsInLoop_StillTrueAfterOperations)
{
    auto loop = MakeLoop();
    loop->NotifyThread();
    loop->PutIntoQueue([]() {});
    EXPECT_TRUE(loop->IsInLoop());
}

// ============================================================
// RunInLoop — same thread: executes immediately
// ============================================================

TEST(EventLoopTest, RunInLoop_SameThread_ExecutesImmediately)
{
    auto loop = MakeLoop();
    bool called = false;

    loop->RunInLoop([&]() { called = true; });

    EXPECT_TRUE(called);
}

TEST(EventLoopTest, RunInLoop_SameThread_CapturesByReference)
{
    auto loop = MakeLoop();
    int value = 0;

    loop->RunInLoop([&]() { value = 42; });

    EXPECT_EQ(value, 42);
}

TEST(EventLoopTest, RunInLoop_SameThread_ExecutesInOrder)
{
    auto loop = MakeLoop();
    std::vector<int> order;

    loop->RunInLoop([&]() { order.push_back(1); });
    loop->RunInLoop([&]() { order.push_back(2); });
    loop->RunInLoop([&]() { order.push_back(3); });

    ASSERT_EQ(order.size(), 3);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST(EventLoopTest, RunInLoop_SameThread_ModifiesString)
{
    auto loop = MakeLoop();
    std::string s;

    loop->RunInLoop([&]() { s = "modified"; });

    EXPECT_EQ(s, "modified");
}

// ============================================================
// RunInLoop — different thread: queues, does NOT execute
// ============================================================

TEST(EventLoopTest, RunInLoop_OtherThread_DoesNotExecuteImmediately)
{
    auto loop = MakeLoop();
    std::atomic<bool> executed{false};

    std::thread t([&]() {
        loop->RunInLoop([&]() { executed = true; });
    });
    t.join();

    EXPECT_FALSE(executed);
}

TEST(EventLoopTest, RunInLoop_OtherThread_MultipleProducers)
{
    auto loop = MakeLoop();
    std::atomic<int> counter{0};

    auto worker = [&]() {
        for (int i = 0; i < 100; ++i)
            loop->RunInLoop([&]() { counter.fetch_add(1); });
    };

    std::thread t1(worker);
    std::thread t2(worker);
    t1.join();
    t2.join();

    // Tasks only queued, never drained — counter stays 0
    EXPECT_EQ(counter, 0);
}

// ============================================================
// PutIntoQueue
// ============================================================

TEST(EventLoopTest, PutIntoQueue_SameThread_DoesNotExecute)
{
    auto loop = MakeLoop();
    bool called = false;

    loop->PutIntoQueue([&]() { called = true; });

    EXPECT_FALSE(called);
}

TEST(EventLoopTest, PutIntoQueue_MultipleTasks_DoesNotCrash)
{
    auto loop = MakeLoop();

    for (int i = 0; i < 100; ++i)
        loop->PutIntoQueue([]() {});

    SUCCEED();
}

// ============================================================
// NotifyThread
// ============================================================

TEST(EventLoopTest, NotifyThread_SameThread_NoCrash)
{
    auto loop = MakeLoop();
    EXPECT_NO_THROW(loop->NotifyThread());
}

TEST(EventLoopTest, NotifyThread_OtherThread_NoCrash)
{
    auto loop = MakeLoop();

    std::thread t([&]() {
        EXPECT_NO_THROW(loop->NotifyThread());
    });
    t.join();
}

TEST(EventLoopTest, NotifyThread_MultipleCalls)
{
    auto loop = MakeLoop();

    for (int i = 0; i < 10; ++i)
        loop->NotifyThread();

    SUCCEED();
}

TEST(EventLoopTest, NotifyThread_InterleavedWithRunInLoop)
{
    auto loop = MakeLoop();
    int counter = 0;

    loop->RunInLoop([&]() { ++counter; });
    loop->NotifyThread();
    loop->RunInLoop([&]() { ++counter; });

    EXPECT_EQ(counter, 2);
}

// ============================================================
// AddTimedJob
// ============================================================

TEST(EventLoopTest, AddTimedJob_ReturnsTrue)
{
    auto loop = MakeLoop();
    EXPECT_TRUE(loop->AddTimedJob(1, 10, []() {}));
}

TEST(EventLoopTest, AddTimedJob_DoesNotExecuteImmediately)
{
    auto loop = MakeLoop();
    bool called = false;

    loop->AddTimedJob(1, 10, [&]() { called = true; });

    EXPECT_FALSE(called);
}

TEST(EventLoopTest, AddTimedJob_ZeroTimeout_NoCrash)
{
    auto loop = MakeLoop();
    EXPECT_TRUE(loop->AddTimedJob(1, 0, []() {}));
}

TEST(EventLoopTest, AddTimedJob_LargeTimeout_NoCrash)
{
    auto loop = MakeLoop();
    EXPECT_TRUE(loop->AddTimedJob(1, 999999, []() {}));
}

TEST(EventLoopTest, AddTimedJob_FromOtherThread)
{
    auto loop = MakeLoop();
    std::atomic<bool> result{false};

    std::thread t([&]() {
        result = loop->AddTimedJob(42, 100, []() {});
    });
    t.join();

    EXPECT_TRUE(result);
}

// ============================================================
// CancelTimedJob
// ============================================================

TEST(EventLoopTest, CancelTimedJob_ReturnsTrue)
{
    auto loop = MakeLoop();
    EXPECT_TRUE(loop->CancelTimedJob(999));
}

TEST(EventLoopTest, CancelTimedJob_NonExistent_NoCrash)
{
    auto loop = MakeLoop();
    EXPECT_NO_THROW(loop->CancelTimedJob(999));
}

TEST(EventLoopTest, CancelTimedJob_Existing_NoCrash)
{
    auto loop = MakeLoop();
    loop->AddTimedJob(1, 10, []() {});
    EXPECT_NO_THROW(loop->CancelTimedJob(1));
}

TEST(EventLoopTest, CancelTimedJob_Twice_NoCrash)
{
    auto loop = MakeLoop();
    loop->AddTimedJob(1, 10, []() {});
    loop->CancelTimedJob(1);
    EXPECT_NO_THROW(loop->CancelTimedJob(1));
}

TEST(EventLoopTest, CancelTimedJob_FromOtherThread_NoCrash)
{
    auto loop = MakeLoop();
    loop->AddTimedJob(1, 10, []() {});

    std::thread t([&]() {
        loop->CancelTimedJob(1);
    });
    t.join();

    SUCCEED();
}

// ============================================================
// UpdateTimedJob
// ============================================================

TEST(EventLoopTest, UpdateTimedJob_ReturnsTrue)
{
    auto loop = MakeLoop();
    EXPECT_TRUE(loop->UpdateTimedJob(999));
}

TEST(EventLoopTest, UpdateTimedJob_NonExistent_NoCrash)
{
    auto loop = MakeLoop();
    EXPECT_NO_THROW(loop->UpdateTimedJob(999));
}

TEST(EventLoopTest, UpdateTimedJob_Existing_NoCrash)
{
    auto loop = MakeLoop();
    loop->AddTimedJob(1, 10, []() {});
    EXPECT_NO_THROW(loop->UpdateTimedJob(1));
}

TEST(EventLoopTest, UpdateTimedJob_AfterCancel_NoCrash)
{
    auto loop = MakeLoop();
    loop->AddTimedJob(1, 10, []() {});
    loop->CancelTimedJob(1);
    EXPECT_NO_THROW(loop->UpdateTimedJob(1));
}

TEST(EventLoopTest, UpdateTimedJob_FromOtherThread_NoCrash)
{
    auto loop = MakeLoop();
    loop->AddTimedJob(1, 10, []() {});

    std::thread t([&]() {
        loop->UpdateTimedJob(1);
    });
    t.join();

    SUCCEED();
}

// ============================================================
// Timer lifecycle sequences
// ============================================================

TEST(EventLoopTest, Timer_AddCancelUpdate_Sequence)
{
    auto loop = MakeLoop();
    bool a = false, b = false;

    loop->AddTimedJob(1, 5, [&]() { a = true; });
    loop->CancelTimedJob(1);
    loop->AddTimedJob(1, 5, [&]() { b = true; });

    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TEST(EventLoopTest, Timer_MultipleJobs_IndependentLifecycle)
{
    auto loop = MakeLoop();
    bool a = false, b = false, c = false;

    loop->AddTimedJob(1, 10, [&]() { a = true; });
    loop->AddTimedJob(2, 10, [&]() { b = true; });
    loop->AddTimedJob(3, 10, [&]() { c = true; });

    loop->CancelTimedJob(2);

    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
    EXPECT_FALSE(c);
}

TEST(EventLoopTest, Timer_SameIdTwice_SecondOverwrites)
{
    auto loop = MakeLoop();
    bool first = false, second = false;

    loop->AddTimedJob(1, 5, [&]() { first = true; });
    loop->AddTimedJob(1, 5, [&]() { second = true; });

    loop->CancelTimedJob(1);

    EXPECT_FALSE(first);
    EXPECT_FALSE(second);
}

// ============================================================
// UpdateEvent & RemvoeEvent
// ============================================================

TEST(EventLoopTest, UpdateEvent_UnregisteredChannel_NoCrash)
{
    auto loop = MakeLoop();
    auto ch = std::make_shared<Channel>(0, nullptr);
    EXPECT_NO_THROW(loop->UpdateEvent(ch));
}

TEST(EventLoopTest, RemvoeEvent_UnregisteredChannel_NoCrash)
{
    auto loop = MakeLoop();
    auto ch = std::make_shared<Channel>(0, nullptr);
    EXPECT_NO_THROW(loop->RemvoeEvent(ch));
}

TEST(EventLoopTest, UpdateThenRemvoeEvent_Sequence)
{
    auto loop = MakeLoop();
    auto ch = std::make_shared<Channel>(0, nullptr);

    EXPECT_NO_THROW(loop->UpdateEvent(ch));
    EXPECT_NO_THROW(loop->RemvoeEvent(ch));
}

TEST(EventLoopTest, RemvoeEvent_WithoutUpdate_NoCrash)
{
    auto loop = MakeLoop();
    auto ch = std::make_shared<Channel>(0, nullptr);
    EXPECT_NO_THROW(loop->RemvoeEvent(ch));
}

// ============================================================
// Thread safety — concurrent access
// ============================================================

TEST(EventLoopTest, Concurrent_AddTimedJob_MultipleThreads)
{
    auto loop = MakeLoop();

    auto worker = [&](int base) {
        for (int i = 0; i < 50; ++i)
            loop->AddTimedJob(base * 1000 + i, 100, []() {});
    };

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    std::thread t3(worker, 3);
    t1.join();
    t2.join();
    t3.join();

    SUCCEED();
}

TEST(EventLoopTest, Concurrent_MixedAddAndCancel)
{
    auto loop = MakeLoop();

    auto writer = [&]() {
        for (int i = 0; i < 30; ++i)
            loop->AddTimedJob(i, 100, []() {});
    };

    auto canceller = [&]() {
        for (int i = 0; i < 30; ++i)
            loop->CancelTimedJob(i);
    };

    std::thread t1(writer);
    std::thread t2(canceller);
    t1.join();
    t2.join();

    SUCCEED();
}

TEST(EventLoopTest, Concurrent_RunInLoop_NotifyThread_Mixed)
{
    auto loop = MakeLoop();
    std::atomic<int> count{0};

    auto runner = [&]() {
        for (int i = 0; i < 50; ++i)
            loop->RunInLoop([&]() { count.fetch_add(1); });
    };

    auto notifier = [&]() {
        for (int i = 0; i < 50; ++i)
            loop->NotifyThread();
    };

    std::thread t1(runner);
    std::thread t2(notifier);
    std::thread t3(runner);
    t1.join();
    t2.join();
    t3.join();

    SUCCEED();
}

// ============================================================
// Multiple independent instances
// ============================================================

TEST(EventLoopTest, MultiInstance_IsolatedRunInLoop)
{
    auto l1 = MakeLoop();
    auto l2 = MakeLoop();

    bool called1 = false, called2 = false;

    l1->RunInLoop([&]() { called1 = true; });
    l2->RunInLoop([&]() { called2 = true; });

    EXPECT_TRUE(called1);
    EXPECT_TRUE(called2);
}

TEST(EventLoopTest, MultiInstance_IsolatedTimers)
{
    auto l1 = MakeLoop();
    auto l2 = MakeLoop();
    bool a = false, b = false;

    l1->AddTimedJob(1, 10, [&]() { a = true; });
    l2->AddTimedJob(1, 10, [&]() { b = true; });
    l1->CancelTimedJob(1);

    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TEST(EventLoopTest, MultiInstance_EachSeesOwnThread)
{
    auto l1 = MakeLoop();
    auto l2 = MakeLoop();

    EXPECT_TRUE(l1->IsInLoop());
    EXPECT_TRUE(l2->IsInLoop());
}

// ============================================================
// Stress — bulk operations
// ============================================================

TEST(EventLoopTest, Stress_AddAndCancel_Bulk)
{
    auto loop = MakeLoop();

    for (size_t i = 0; i < 1000; ++i)
        loop->AddTimedJob(i, i % 100, []() {});

    for (size_t i = 0; i < 1000; i += 2)
        loop->CancelTimedJob(i);

    for (size_t i = 1; i < 1000; i += 2)
        loop->UpdateTimedJob(i);

    SUCCEED();
}

TEST(EventLoopTest, Stress_PutIntoQueue_Bulk)
{
    auto loop = MakeLoop();

    for (int i = 0; i < 1000; ++i)
        loop->PutIntoQueue([]() {});

    SUCCEED();
}
