
#pragma once

/*
    - 字符串工具类
    - 成员 :
            1) 字符串分割
            2) 字符串编码 , 字符串解码

*/

#include <iostream>
#include <string>
#include <vector>

class StringUtil
{
public:
    static int Spilt(const std::string &src , const std::string& sep , std::vector<std::string>& array);

    static std::string UrlEncode(const std::string& raw);
    static std::string UrlDecode(const std::string& raw);

};