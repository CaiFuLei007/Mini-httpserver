#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <functional>

#include "tcp_server/timerwheel.h"

// ============================================================
// TimeTask — Constructor & Initial State
// ============================================================

TEST(TimeTaskTest, Constructor_DoesNotExecuteTask)
{
    bool called = false;
    TimeTask task(1, 10, [&called]() { called = true; });

    EXPECT_FALSE(called);
}

TEST(TimeTaskTest, Timeout_ReturnsStoredValue)
{
    TimeTask task1(1, 100, []() {});
    EXPECT_EQ(task1.Timeout(), 100);

    TimeTask task2(2, 0, []() {});
    EXPECT_EQ(task2.Timeout(), 0);

    TimeTask task3(3, 9999, []() {});
    EXPECT_EQ(task3.Timeout(), 9999);
}

// ============================================================
// TimeTask — Destructor Behaviour
// ============================================================

TEST(TimeTaskTest, Destructor_ExecutesTaskWhenNotCancelled)
{
    bool called = false;
    {
        TimeTask task(1, 5, [&called]() { called = true; });
        EXPECT_FALSE(called);
    }
    EXPECT_TRUE(called);
}

TEST(TimeTaskTest, Destructor_DoesNotExecuteWhenCancelled)
{
    bool called = false;
    {
        TimeTask task(1, 5, [&called]() { called = true; });
        task.Cancel();
    }
    EXPECT_FALSE(called);
}

TEST(TimeTaskTest, Destructor_HandlesNullFunction)
{
    {
        TimeTask task(1, 5, nullptr);
    }
    {
        TimeTask task(2, 10, std::function<void()>());
    }
    SUCCEED();
}

TEST(TimeTaskTest, Destructor_CallbackCapturesByReference)
{
    int value = 0;
    {
        TimeTask task(1, 5, [&value]() { value = 42; });
    }
    EXPECT_EQ(value, 42);
}

// ============================================================
// TimeTask — Cancel
// ============================================================

TEST(TimeTaskTest, Cancel_IsIdempotent)
{
    bool called = false;
    {
        TimeTask task(1, 5, [&called]() { called = true; });
        task.Cancel();
        task.Cancel();
        task.Cancel();
    }
    EXPECT_FALSE(called);
}

TEST(TimeTaskTest, Cancel_OnlyAffectsTargetTask)
{
    bool called1 = false;
    bool called2 = false;

    {
        TimeTask task1(1, 5, [&called1]() { called1 = true; });
        TimeTask task2(2, 5, [&called2]() { called2 = true; });

        task1.Cancel();
    }

    EXPECT_FALSE(called1);
    EXPECT_TRUE(called2);
}

TEST(TimeTaskTest, Cancel_AllTasksIndependent)
{
    int count = 0;

    {
        TimeTask t1(1, 5, [&count]() { ++count; });
        TimeTask t2(2, 5, [&count]() { ++count; });
        TimeTask t3(3, 5, [&count]() { ++count; });

        t2.Cancel();
    }

    EXPECT_EQ(count, 2);
}

// ============================================================
// TimeTask — Execution Order on Destruction
// ============================================================

TEST(TimeTaskTest, Destructor_StackUnwindingOrder)
{
    // Tasks destroyed in reverse construction order (stack unwinding)
    std::vector<int> order;

    {
        TimeTask t1(1, 5, [&order]() { order.push_back(1); });
        TimeTask t2(2, 5, [&order]() { order.push_back(2); });
        TimeTask t3(3, 5, [&order]() { order.push_back(3); });
    }

    ASSERT_EQ(order.size(), 3);
    EXPECT_EQ(order[0], 3);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 1);
}

// ============================================================
// TimeTask — Cancelled Tasks in Mixed Scope
// ============================================================

TEST(TimeTaskTest, Cancel_MixedCancelledAndActive)
{
    int count = 0;

    {
        TimeTask t1(1, 5, [&count]() { ++count; });
        t1.Cancel();

        TimeTask t2(2, 5, [&count]() { ++count; });

        TimeTask t3(3, 5, [&count]() { ++count; });
        t3.Cancel();

        TimeTask t4(4, 5, [&count]() { ++count; });
    }

    EXPECT_EQ(count, 2);
}

// ============================================================
// TimerWheel — CancelTask (works via tasks_ map, no wheel
// dependency beyond construction)
// ============================================================

TEST(TimerWheelTest, CancelTask_NonExistentId_DoesNotCrash)
{
    TimerWheel tw;
    EXPECT_NO_THROW(tw.CancelTask(999));
}

// ============================================================
// TimerWheel — UpdateTask on missing id
// ============================================================

TEST(TimerWheelTest, UpdateTask_NonExistentId_DoesNotCrash)
{
    TimerWheel tw;
    EXPECT_NO_THROW(tw.UpdateTask(999));
}

// ============================================================
// TimerWheel — SetTask + CancelTask round-trip
// ============================================================

TEST(TimerWheelTest, SetTaskAndCancel_TaskDoesNotExecute)
{
    TimerWheel tw;
    bool called = false;

    tw.SetTask(1, 1, [&called]() { called = true; });
    tw.CancelTask(1);

    EXPECT_FALSE(called);
}

// ============================================================
// TimerWheel — SetTask with zero timeout (executes on current
// tick when HandleAllTask fires on that slot)
// ============================================================

TEST(TimerWheelTest, SetTask_WithTimeout_StoresWithoutExecuting)
{
    TimerWheel tw;
    bool called = false;

    tw.SetTask(42, 10, [&called]() { called = true; });

    // Task is stored but should not execute until the wheel slot is
    // cleared (which happens when the timerfd fires and advances the
    // tick to that slot). So immediately after SetTask, no execution.
    EXPECT_FALSE(called);
}

// ============================================================
// TimerWheel — Multiple SetTask / CancelTask sequences
// ============================================================

TEST(TimerWheelTest, SetMultipleTasks_CancelSome)
{
    TimerWheel tw;
    bool a = false, b = false, c = false;

    tw.SetTask(1, 5, [&a]() { a = true; });
    tw.SetTask(2, 5, [&b]() { b = true; });
    tw.SetTask(3, 5, [&c]() { c = true; });

    tw.CancelTask(2);

    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
    EXPECT_FALSE(c);
}

// ============================================================
// TimerWheel — UpdateTask re-inserts task into wheel
// ============================================================

TEST(TimerWheelTest, UpdateTask_ExistingTask_DoesNotCrash)
{
    TimerWheel tw;

    tw.SetTask(1, 5, []() {});
    EXPECT_NO_THROW(tw.UpdateTask(1));
}

// ============================================================
// TimerWheel — Cancel already-cancelled task
// ============================================================

TEST(TimerWheelTest, CancelTask_TwiceOnSameId)
{
    TimerWheel tw;
    tw.SetTask(1, 5, []() {});

    tw.CancelTask(1);
    EXPECT_NO_THROW(tw.CancelTask(1));
}

// ============================================================
// TimerWheel — Cancel then Update
// ============================================================

TEST(TimerWheelTest, CancelThenUpdate)
{
    TimerWheel tw;
    tw.SetTask(1, 5, []() {});

    tw.CancelTask(1);
    EXPECT_NO_THROW(tw.UpdateTask(1));
}

// ============================================================
// TimerWheel — SetTask overwrites previous task with same id
// (tasks_ uses the same key)
// ============================================================

TEST(TimerWheelTest, SetTask_SameIdTwice_SecondOverwrites)
{
    TimerWheel tw;
    bool first = false, second = false;

    tw.SetTask(1, 5, [&first]() { first = true; });
    tw.SetTask(1, 5, [&second]() { second = true; });

    // Cancel the current task for id=1 — only the second callback
    // should be associated with it
    tw.CancelTask(1);

    EXPECT_FALSE(first);
    EXPECT_FALSE(second);
}

// ============================================================
// TimerWheel — Ready starts the timer (integration smoke test)
// ============================================================

TEST(TimerWheelTest, Ready_DoesNotCrash)
{
    TimerWheel tw;
    EXPECT_NO_THROW(tw.Ready());
}
