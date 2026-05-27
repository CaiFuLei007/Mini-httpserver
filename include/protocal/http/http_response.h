
#pragma once

/*
    - 存储响应信息
    - 成员 :
            1) 请求行 : 协议版本  状态码  状态码描述
            2) 请求头
            3) 请求正文
            4) 是否属于长连接
            5) 是否进行重定向
            6) 重定向地址

*/

#include <iostream>
#include <string>
#include <unordered_map>

class HttpResponse
{
private:
    std::string version_;
    int response_code_;

    std::unordered_map<std::string , std::string > params_;
    std::unordered_map<std::string , std::string > headers_;

    std::string body_;
    bool keepalive_;

    bool is_redirect_;
    std::string redirect_url_;
public:
    HttpResponse()
    :version_("HTTP/1.1") ,
    response_code_(200) , 
    keepalive_(true) , 
    is_redirect_(false)
    {}
    
    void SetVersion(const std::string &version)
    {
        version_ = version;
    }

    std::string GetVersion()
    {
        return version_;
    }

    void SetResponseCode(int code)
    {
        response_code_ = code;
    }

    int GetResponseCode()
    {
        return response_code_;
    }

    void SetParam(const std::string& key , const std::string &value)
    {
        params_[key] = value;
    }

    void SetHeader(const std::string& key , const std::string& value)
    {
        headers_[key] = value;
    }

    void SetKeepAlive()
    {
        keepalive_ = true;
    }

    bool KeepAlive()
    {
        return keepalive_;
    }

    void SetBody(const std::string& body , const std::string &file_type)
    {
        body_ = body;
        SetHeader("Content-Type" , file_type);
    }

    std::string GetBody()
    {
        return body_;
    }

    void AppendBody(const std::string &data)
    {
        body_ += data;
    }

    std::unordered_map<std::string , std::string>& GetAllHeaders()
    {
        return headers_;
    }
    
    std::string GetParam(const std::string &key);
    std::string GetHeader(const std::string& key);
    size_t ContentLength();

    void SetRedirect(const std::string& url)
    {
        is_redirect_ = true;
        redirect_url_ = url;
    }
};