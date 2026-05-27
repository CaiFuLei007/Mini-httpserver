
#pragma once

/*
    - 作为连接的上下文 , 用于解析数据  , 获取 Request 请求
    - 成员 : 
            1) Request 请求体
            2) 状态码
            3) 是否解析错误
            4) 解析状态
    - 接口 : 
            1) 解析请求行 , 请求头 , 请求体
            2) 获取状态码 , 是否解析错误
            3) 获取完整请求
            4) 解析请求

*/

#include "protocal/http/http_resquest.h"
#include "base/buffer.h"

#include <memory>

enum class ContextStatus
{
    PARSELINE , 
    PARSEHEADER , 
    PARSEBODY , 
    FINISH ,
    ERROR  
};

class HttpContext
{
private:
    std::shared_ptr<HttpRequest> request_;
    int status_code_;
    bool is_error_;
    enum ContextStatus parse_stats_;

private:    
    bool ParseLine(const std::string& line);
    bool ParseHeader(const std::string& data);
    bool ParseBody(const std::string& data);

    bool HandleLine(std::shared_ptr<Buffer> buf);
    bool HandleHeader(std::shared_ptr<Buffer> buf);
    bool HandleBody(std::shared_ptr<Buffer> buf);

public:
    HttpContext()
    :request_(std::make_shared<HttpRequest>()) ,
    status_code_(200) , 
    is_error_(false) , 
    parse_stats_(ContextStatus::PARSELINE)
    {}

    bool Good()
    {
        return !is_error_;
    }

    std::shared_ptr<HttpRequest> GetRequest()
    {
        return request_;
    }

    ContextStatus GetParseStatus()
    {
        return parse_stats_;
    }

    int GetStatusCode()
    {
        return status_code_;
    }

    void Clear()
    {
        request_->Clear();
        status_code_ = 200;
        is_error_ = false;
        parse_stats_ = ContextStatus::PARSELINE;
    }


    void Parse(std::shared_ptr<Buffer> buf);
};