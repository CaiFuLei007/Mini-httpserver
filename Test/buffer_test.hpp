#pragma once

#include <gtest/gtest.h>
#include <string>
#include <cstring>

#include "base/buffer.h"

// ============================================================
// Constructor & Initial State
// ============================================================

TEST(BufferTest, Constructor_InitialState)
{
    Buffer buf;
    EXPECT_EQ(buf.Size(), 0);
    EXPECT_NE(buf.ReadAddr(), nullptr);
    EXPECT_NE(buf.WriteAddr(), nullptr);
}

// ============================================================
// WriteAndPush (char* version)
// ============================================================

TEST(BufferTest, WriteAndPush_CharPointer)
{
    Buffer buf;
    const char* data = "Hello";
    buf.WriteAndPush(const_cast<char*>(data), 5);
    EXPECT_EQ(buf.Size(), 5);
}

TEST(BufferTest, WriteAndPush_CharPointer_Empty)
{
    Buffer buf;
    char data[] = "";
    buf.WriteAndPush(data, 0);
    EXPECT_EQ(buf.Size(), 0);
}

// ============================================================
// WriteAndPush (string version)
// ============================================================

TEST(BufferTest, WriteAndPush_String)
{
    Buffer buf;
    std::string data = "Hello";
    buf.WriteAndPush(data);
    EXPECT_EQ(buf.Size(), 5);
}

TEST(BufferTest, WriteAndPush_String_Empty)
{
    Buffer buf;
    std::string data = "";
    buf.WriteAndPush(data);
    EXPECT_EQ(buf.Size(), 0);
}

// ============================================================
// ReadAndPop (char* version)
// ============================================================

TEST(BufferTest, ReadAndPop_CharPointer)
{
    Buffer buf;
    const char* data = "Hello";
    buf.WriteAndPush(const_cast<char*>(data), 5);

    char out[16] = {0};
    size_t ret = buf.ReadAndPop(out, 5);
    EXPECT_EQ(ret, 5);
    EXPECT_STREQ(out, "Hello");
    EXPECT_EQ(buf.Size(), 0);
}

TEST(BufferTest, ReadAndPop_CharPointer_Partial)
{
    Buffer buf;
    const char* data = "HelloWorld";
    buf.WriteAndPush(const_cast<char*>(data), 10);

    char out[16] = {0};
    size_t ret = buf.ReadAndPop(out, 5);
    EXPECT_EQ(ret, 5);
    EXPECT_EQ(std::string(out, 5), "Hello");
    EXPECT_EQ(buf.Size(), 5);
}

TEST(BufferTest, ReadAndPop_CharPointer_EmptyBuffer)
{
    Buffer buf;
    char out[16] = {0};
    size_t ret = buf.ReadAndPop(out, 5);
    EXPECT_EQ(ret, 0);
}

// ============================================================
// ReadAndPop (string version)
// ============================================================

TEST(BufferTest, ReadAndPop_String)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello"));

    std::string out;
    size_t ret = buf.ReadAndPop(out);
    EXPECT_EQ(ret, 5);
    EXPECT_EQ(out, "Hello");
    EXPECT_EQ(buf.Size(), 0);
}

TEST(BufferTest, ReadAndPop_String_EmptyBuffer)
{
    Buffer buf;
    std::string out;
    size_t ret = buf.ReadAndPop(out);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(out, "");
}

// ============================================================
// Read (non-pop, char* version)
// ============================================================

TEST(BufferTest, Read_CharPointer_NoMove)
{
    Buffer buf;
    const char* data = "Hello";
    buf.WriteAndPush(const_cast<char*>(data), 5);

    char out[16] = {0};
    size_t ret = buf.Read(out, 5);
    EXPECT_EQ(ret, 5);
    EXPECT_STREQ(out, "Hello");
    // Size should remain unchanged (no pop)
    EXPECT_EQ(buf.Size(), 5);
}

// ============================================================
// Read (non-pop, string version)
// ============================================================

TEST(BufferTest, Read_String_NoMove)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello"));

    std::string out;
    size_t ret = buf.Read(out);
    EXPECT_EQ(ret, 5);
    EXPECT_EQ(out, "Hello");
    EXPECT_EQ(buf.Size(), 5);
}

// ============================================================
// Write (non-push, char* version)
// ============================================================

TEST(BufferTest, Write_CharPointer_NoMove)
{
    Buffer buf;
    const char* data = "Hello";
    buf.Write(const_cast<char*>(data), 5);
    // Size unchanged because write_index_ not moved
    EXPECT_EQ(buf.Size(), 0);

    buf.MoveWriteAddr(5);
    EXPECT_EQ(buf.Size(), 5);
}

// ============================================================
// Write (non-push, string version)
// ============================================================

TEST(BufferTest, Write_String_NoMove)
{
    Buffer buf;
    buf.Write(std::string("Hello"));
    EXPECT_EQ(buf.Size(), 0);

    buf.MoveWriteAddr(5);
    EXPECT_EQ(buf.Size(), 5);
}

// ============================================================
// MoveReadAddr / MoveWriteAddr
// ============================================================

TEST(BufferTest, MoveReadAddr)
{
    Buffer buf;
    buf.WriteAndPush(std::string("HelloWorld"));

    buf.MoveReadAddr(5);
    EXPECT_EQ(buf.Size(), 5);

    std::string out;
    buf.ReadAndPop(out);
    EXPECT_EQ(out, "World");
}

TEST(BufferTest, MoveWriteAddr)
{
    Buffer buf;
    const char* data = "Hello";
    buf.Write(const_cast<char*>(data), 5);
    EXPECT_EQ(buf.Size(), 0);

    buf.MoveWriteAddr(5);
    EXPECT_EQ(buf.Size(), 5);
}

// ============================================================
// GetLine
// ============================================================

TEST(BufferTest, GetLine_WithNewline)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello\nWorld"));

    std::string line = buf.GetLine();
    EXPECT_EQ(line, "Hello");
    // Size unchanged
    EXPECT_EQ(buf.Size(), 11);
}

TEST(BufferTest, GetLine_NoNewline)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello"));

    std::string line = buf.GetLine();
    EXPECT_EQ(line, "");
}

TEST(BufferTest, GetLine_EmptyBuffer)
{
    Buffer buf;
    std::string line = buf.GetLine();
    EXPECT_EQ(line, "");
}

TEST(BufferTest, GetLine_MultipleLines)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Line1\nLine2\nLine3\n"));

    std::string line1 = buf.GetLine();
    EXPECT_EQ(line1, "Line1");
}

// ============================================================
// GetLineAndPop
// ============================================================

TEST(BufferTest, GetLineAndPop_Basic)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello\nWorld"));

    std::string line = buf.GetLineAndPop();
    EXPECT_EQ(line, "Hello");
    EXPECT_EQ(buf.Size(), 5);

    // Remaining data should be "World"
    std::string rest;
    buf.ReadAndPop(rest);
    EXPECT_EQ(rest, "World");
}

TEST(BufferTest, GetLineAndPop_AllLines)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Line1\nLine2\nLine3\n"));

    EXPECT_EQ(buf.GetLineAndPop(), "Line1");
    EXPECT_EQ(buf.Size(), 12);

    EXPECT_EQ(buf.GetLineAndPop(), "Line2");
    EXPECT_EQ(buf.Size(), 6);

    // "Line3\n" remains — GetLineAndPop reads to '\n' and pops
    EXPECT_EQ(buf.GetLineAndPop(), "Line3");
    EXPECT_EQ(buf.Size(), 0);
}

// ============================================================
// Capacity Expansion
// ============================================================

TEST(BufferTest, WriteBeyondDefaultCapacity)
{
    Buffer buf;
    std::string big_data(100, 'A');
    buf.WriteAndPush(big_data);

    EXPECT_EQ(buf.Size(), 100);

    std::string out;
    buf.ReadAndPop(out);
    EXPECT_EQ(out, big_data);
}

TEST(BufferTest, WriteExactCapacity)
{
    Buffer buf; // default capacity 16
    std::string data(16, 'X');
    buf.WriteAndPush(data);
    EXPECT_EQ(buf.Size(), 16);

    std::string out;
    buf.ReadAndPop(out);
    EXPECT_EQ(out, data);
}

// ============================================================
// Data Shifting (front empty space reuse)
// ============================================================

TEST(BufferTest, DataShift_AfterPartialRead)
{
    Buffer buf;
    buf.WriteAndPush(std::string("HelloWorld")); // size = 10

    // Read 5 bytes to move read_index_
    char out[16] = {0};
    buf.ReadAndPop(out, 5);
    EXPECT_EQ(buf.Size(), 5);

    // Now write more data — triggers shift if front space is reused
    buf.WriteAndPush(std::string("ABCDE")); // 5 + 5 = 10
    EXPECT_EQ(buf.Size(), 10);

    std::string rest;
    buf.ReadAndPop(rest);
    EXPECT_EQ(rest, "WorldABCDE");
}

TEST(BufferTest, DataShift_MultipleTimes)
{
    Buffer buf;
    buf.WriteAndPush(std::string("AAAAA")); // size = 5
    EXPECT_EQ(buf.Size(), 5);

    char out[16] = {0};
    buf.ReadAndPop(out, 5);                // size = 0, read_index moves
    EXPECT_EQ(buf.Size(), 0);

    buf.WriteAndPush(std::string("BBBBB")); // size = 5
    EXPECT_EQ(buf.Size(), 5);

    buf.ReadAndPop(out, 5);                // size = 0
    EXPECT_EQ(buf.Size(), 0);

    buf.WriteAndPush(std::string("CCCCC")); // should trigger shift
    EXPECT_EQ(buf.Size(), 5);

    std::string rest;
    buf.ReadAndPop(rest);
    EXPECT_EQ(rest, "CCCCC");
}

// ============================================================
// Mixed Operations
// ============================================================

TEST(BufferTest, WriteAndReadSequence)
{
    Buffer buf;
    buf.WriteAndPush(std::string("AAA"));
    buf.WriteAndPush(std::string("BBB"));
    EXPECT_EQ(buf.Size(), 6);

    std::string out;
    buf.ReadAndPop(out);
    EXPECT_EQ(out, "AAABBB");
    EXPECT_EQ(buf.Size(), 0);
}

TEST(BufferTest, InterleavedReadWrite)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello"));

    char out1[16] = {0};
    buf.ReadAndPop(out1, 3);
    EXPECT_EQ(std::string(out1, 3), "Hel");
    EXPECT_EQ(buf.Size(), 2);

    buf.WriteAndPush(std::string("World"));
    EXPECT_EQ(buf.Size(), 7);

    std::string out2;
    buf.ReadAndPop(out2);
    EXPECT_EQ(out2, "loWorld");
}

// ============================================================
// Edge Cases
// ============================================================

TEST(BufferTest, ReadMoreThanAvailable)
{
    Buffer buf;
    buf.WriteAndPush(std::string("abc"));

    char out[16] = {0};
    size_t ret = buf.ReadAndPop(out, 10);
    EXPECT_EQ(ret, 3);
    EXPECT_EQ(std::string(out, 3), "abc");
    EXPECT_EQ(buf.Size(), 0);
}

TEST(BufferTest, WriteZeroSize)
{
    Buffer buf;
    buf.WriteAndPush(const_cast<char*>(""), 0);
    EXPECT_EQ(buf.Size(), 0);

    buf.WriteAndPush(std::string(""));
    EXPECT_EQ(buf.Size(), 0);
}

TEST(BufferTest, LargeDataStress)
{
    Buffer buf;
    std::string chunk(1024, 'M');
    for (int i = 0; i < 10; ++i)
    {
        buf.WriteAndPush(chunk);
    }
    EXPECT_EQ(buf.Size(), 10240);

    std::string out;
    buf.ReadAndPop(out);
    EXPECT_EQ(out.size(), 10240);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(out.substr(i * 1024, 1024), chunk);
    }
}

TEST(BufferTest, RepeatedSmallWrites)
{
    Buffer buf;
    char c = 'A';
    for (int i = 0; i < 100; ++i)
    {
        buf.WriteAndPush(std::string(1, c));
        c = (c == 'Z') ? 'A' : c + 1;
    }
    EXPECT_EQ(buf.Size(), 100);

    std::string out;
    buf.ReadAndPop(out);
    EXPECT_EQ(out.size(), 100);
    EXPECT_EQ(out[0], 'A');
    EXPECT_EQ(out[25], 'Z');
}

// ============================================================
// ReadAddr / WriteAddr correctness
// ============================================================

TEST(BufferTest, AddrDistanceEqualsSize)
{
    Buffer buf;
    EXPECT_EQ(buf.WriteAddr() - buf.ReadAddr(), 0);

    buf.WriteAndPush(std::string("Hello"));
    EXPECT_EQ(buf.WriteAddr() - buf.ReadAddr(), 5);

    buf.MoveReadAddr(2);
    EXPECT_EQ(buf.WriteAddr() - buf.ReadAddr(), 3);
}

// ============================================================
// GetLine with '\n' at end of buffer
// ============================================================

TEST(BufferTest, GetLine_NewlineAtEnd)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello\n"));

    std::string line = buf.GetLine();
    EXPECT_EQ(line, "Hello");

    buf.GetLineAndPop();
    EXPECT_EQ(buf.Size(), 0);
}

TEST(BufferTest, GetLineAndPop_NoNewline)
{
    Buffer buf;
    buf.WriteAndPush(std::string("Hello"));

    std::string line = buf.GetLineAndPop();
    EXPECT_EQ(line, "");
    EXPECT_EQ(buf.Size(), 5); // nothing popped
}