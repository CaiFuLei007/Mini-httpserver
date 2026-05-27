
#pragma once


/*
    - 存储 HTTP Request 请求
    - 成员 : 
            1) 请求行 : 请求方法 , 请求 Path , 协议版本
            2) 请求参数  ,请求头
            3) 请求正文
            4) 是否属于长连接
    - 方法 :    
            1) 设置 , 获取各个参数
            2) 正文长度 , 是否属于长连接
*/

#include <iostream>
#include <string>
#include <unordered_map>

class HttpRequest
{
private:
    std::string method_;
    std::string path_;
    std::string version_;
    
    std::unordered_map<std::string , std::string > params_;
    std::unordered_map<std::string , std::string > headers_;

    std::string body_;
    bool keepalive_;
public:
    HttpRequest()
    :keepalive_(false)
    {}
    
    void SetMethod(const std::string &method)
    {
        method_ = method;
    }

    std::string GetMethod()
    {
        return method_;
    }

    void SetPath(const std::string& path)
    {
        path_ = path;
    }

    std::string GetPath()
    {
        return path_;
    }

    void SetVersion(const std::string& version)
    {
        version_ = version;
    }

    std::string GetVersion()
    {
        return version_;
    }

    void SetParam(const std::string& key , const std::string &value)
    {
        params_[key] = value;
    }

    void SetHeader(const std::string& key , const std::string& value)
    {
        headers_[key] = value;
    }

    void SetBody(const std::string& body)
    {
        body_ = body;
    }

    std::string GetBody()
    {
        return body_;
    }

    void AppendBody(const std::string &data)
    {
        body_ += data;
    }

    void SetKeepAlive()
    {
        keepalive_ = true;
    }

    bool KeepAlive()
    {
        return keepalive_;
    }    

    void Clear()
    {
        method_ = path_ = version_ = "" ;
        params_.clear();
        headers_.clear();

        body_.clear();
        keepalive_ = false;
    }
    
    std::string GetParam(const std::string &key);
    std::string GetHeader(const std::string& key);
    size_t ContentLength();

    size_t BodyLength()
    {
        return body_.size();
    }
};