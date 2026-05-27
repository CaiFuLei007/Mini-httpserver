
#pragma once

/*
    - 搭建 HTTP 服务器
    - 成员 : 
            1) 路由表 : 存储 GET POST PUT DELETE 处理方法
            2) 根目录
            3) TcpServer
    - 接口 :
            1) 回调 , message处理方法 , 新连接到来得的处理方法
            2) 设置从属线程个数
            3) 是否开启超时自动释放
            4) 启动服务器
*/

#include "protocal/http/http_response.h"
#include "protocal/http/http_resquest.h"
#include "connection.h"
#include "tcp_server.h"

#include <iostream>
#include <unordered_map>
#include <regex>



class HttpServer
{
        using Handle = std::function<void(std::shared_ptr<HttpRequest> , std::shared_ptr<HttpResponse>)>;
private:
        std::vector<std::pair<std::regex , Handle> > get_route_;
        std::vector<std::pair<std::regex , Handle> > post_route_;
        std::vector<std::pair<std::regex , Handle> > put_route_;
        std::vector<std::pair<std::regex , Handle> > delete_route_;

        std::string base_dir_;
        TcpServer server_;
private:
        void MesssageCallback(std::shared_ptr<Connection> conn , std::shared_ptr<Buffer> buf);
        void NewConnectCallback(std::shared_ptr<Connection> conn );

        void HandleError(std::shared_ptr<HttpResponse> response);

        bool IsFileRequest(std::shared_ptr<HttpRequest> request );
        void HandleFileRequest(std::shared_ptr<HttpRequest> request , std::shared_ptr<HttpResponse> response);

        void Dispatcher(std::shared_ptr<HttpRequest> request , std::shared_ptr<HttpResponse> response ,std::vector<std::pair<std::regex , Handle> > route);
        void Route(std::shared_ptr<HttpRequest> request , std::shared_ptr<HttpResponse> response);
public:
        HttpServer(uint16_t port);

        void SetBaseDir(const std::string& dir) { base_dir_ = dir; }

        void Get(const std::string& pattern , Handle handle) { get_route_.push_back({std::regex(pattern) , handle}); }
        void Post(const std::string& pattern , Handle handle) { post_route_.push_back({std::regex(pattern) , handle}); }
        void Put(const std::string& pattern , Handle handle) { put_route_.push_back({std::regex(pattern) , handle}); }
        void Delete(const std::string& pattern , Handle handle) { delete_route_.push_back({std::regex(pattern) , handle}); }

        void SetThreadCount(int count);
        void SetSelfRelease(size_t timeout);

        void Run();
};