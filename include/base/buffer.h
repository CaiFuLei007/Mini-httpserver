
#pragma once


/*
    - 对 Buffer 缓冲区进行封装 , 暂存网络数据
    - 成员 : 
            vector<char> 缓冲区
            read_index , write_index 两个下标指针
    - 接口 : 
            1) 获取读取和写入地址
            2) 插入数据 , 读取数据 (两个版本 : 移动指针位置 , 不移动指针位置)
            3) 获取一行数据
            4) 空间不够需要扩容

*/

#include <vector>
#include <iostream>

class Buffer
{
    static constexpr size_t kDefaultCapcity = 16;
private:
    std::vector<char> buffer_;
    size_t read_index_ ;
    size_t write_index_;

private:
    void CheckCapcity(size_t additional_space);

    size_t FrontEmptySpace() const
    {
        return read_index_;
    }

    size_t EndEmptySpace() const
    {
        return buffer_.size() - write_index_;
    }

public: 

    Buffer()
    :buffer_(kDefaultCapcity) , 
    read_index_(0) , 
    write_index_(0)
    {}

    const char* ReadAddr() const
    {
        return &buffer_[0] + read_index_;
    }

    char* WriteAddr()
    {
        return &buffer_[0] + write_index_;
    }

    size_t Size() const
    {
        return write_index_ - read_index_;
    }

    void MoveReadAddr(size_t n)
    {
        read_index_ += n;
    }

    void MoveWriteAddr(size_t n)
    {
        write_index_ += n;
    }

    size_t Read(char* buf , size_t size);
    size_t Read(std::string &buf);
    size_t ReadAndPop(char* buf , size_t size);
    size_t ReadAndPop(std::string &buf);

    bool Write(char* buf , size_t size);
    bool Write(const std::string& buf);
    bool WriteAndPush(char* buf , size_t size);
    bool WriteAndPush(const std::string& buf);

    std::string GetLine();
    std::string GetLineAndPop();
};
