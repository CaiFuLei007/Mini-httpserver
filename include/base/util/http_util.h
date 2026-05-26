
#pragma once


/*
    - HTTP 工具类
    - 接口:
            1) 根据状态码 获取状态码描述

*/

#include <iostream>
#include <string>

class HttpUtil
{
public:
    static std::string GetResponseCodeDes(int response_code);
};