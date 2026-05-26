
#include "base/buffer.h"

#include <cstring>
#include <algorithm>

void Buffer::CheckCapcity(size_t additional_space)
{
    size_t end_empty_space = EndEmptySpace() , front_empty_space = FrontEmptySpace();
    size_t sz = Size();
    if(end_empty_space >= additional_space)
    {
        return;
    }
    else if(end_empty_space + front_empty_space >= additional_space)
    {
        // 将数据整体向前移动
        std::copy(buffer_.begin() + read_index_ , buffer_.begin() + write_index_ , buffer_.begin());
        read_index_ = 0 , write_index_ = sz;
        return ;
    }

    // 扩容
    buffer_.resize(write_index_ + additional_space);
    return ;
}


size_t Buffer::Read(char* buf , size_t size)
{
    size_t read_sz = std::min(Size() , size);
    memset((void*)buf , 0 ,size*sizeof(char));

    std::copy(ReadAddr() , ReadAddr() + read_sz , buf);

    return read_sz;
}

size_t Buffer::Read(std::string &buf)
{
    size_t read_sz = Size();
    buf.resize(read_sz);

    std::copy(ReadAddr() , ReadAddr() + read_sz , &buf[0]);

    return read_sz;
}

size_t Buffer::ReadAndPop(char* buf , size_t size)
{
    size_t ret = Read(buf , size);
    MoveReadAddr(ret);
    return ret;
}

size_t Buffer::ReadAndPop(std::string &buf)
{
    size_t ret = Read(buf);
    MoveReadAddr(ret);
    return ret;
}

bool Buffer::Write(char* buf , size_t size)
{
    CheckCapcity(size);

    std::copy(&buf[0] , &buf[0] + size , WriteAddr());
    return true;
}

bool Buffer::Write(const std::string& buf)
{
    size_t sz = buf.size();
    CheckCapcity(sz);

    std::copy(&buf[0] , &buf[0] + sz , WriteAddr());
    return true;
}

bool Buffer::WriteAndPush(char* buf , size_t size)
{
    Write(buf , size);
    MoveWriteAddr(size);

    return true;
}

bool Buffer::WriteAndPush(const std::string& buf)
{
    Write(buf);
    MoveWriteAddr(buf.size());

    return true;
}


std::string Buffer::GetLine()
{
    auto pos = std::find(ReadAddr() , static_cast<const char*>(WriteAddr()) , '\n');
    if(pos == WriteAddr())
    {
        return "";
    }
    return std::string(ReadAddr() , pos);
}
std::string Buffer::GetLineAndPop()
{
    std::string ret = GetLine();
    if(!ret.empty())
    {
        MoveReadAddr(ret.size() + 1);
    }
    return ret;
}