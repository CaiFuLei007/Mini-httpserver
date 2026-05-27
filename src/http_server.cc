
#include "http_server.h"
#include "protocal/http/http_context.h"
#include "base/util/file_util.h"
#include "base/util/http_util.h"

#include <sstream>


void HttpServer::HandleError(std::shared_ptr<HttpResponse> response)
{
    // 根据错误码 , 来组织错误
    std::string body ;
    std::string path = base_dir_ + "/404.html" ;
    FileUtil::Read(path, body);

    response->SetBody(body , FileUtil::GetFileType(path));
    return ;
}

void SendResponse(std::shared_ptr<Connection> conn , std::shared_ptr<HttpResponse> response)
{
    int response_code = response->GetResponseCode();

    std::ostringstream oss;
    oss << response->GetVersion() << " " << response_code << " " << HttpUtil::GetResponseCodeDes(response_code) << "\r\n";
    for(auto& [key , value] : response->GetAllHeaders())
    {
        oss << key << ": " << value << "\r\n";
    }
    if(response->KeepAlive())
    {
        oss << "Connection: keep-alive" << "\r\n";
    }
    oss << "Content-Length: " << response->ContentLength() << "\r\n";
    oss << "\r\n";
    oss << response->GetBody();

    conn->SendMessage(oss.str());
}

bool HttpServer::IsFileRequest(std::shared_ptr<HttpRequest> request)
{
    // 1. 有base_dir
    // 2. GET 或 HEAD 请求
    // 3. path 是合法路径
    // 4. 请求的资源必须存在,且是一个普通文件

    if(base_dir_.empty())
    {
        return false;
    }

    std::string method = request->GetMethod();
    if(method != "GET" && method != "HEAD")
    {
        return false;
    }

    std::string path = base_dir_ + request->GetPath();
    if(!FileUtil::IsValidPath(path))
    {
        return false;
    }

    if(path.back() == '/')
        path += "index.html";

    if(!FileUtil::IsRegularFile(path))
    {
        return false;
    }
    return true;
}


void HttpServer::HandleFileRequest(std::shared_ptr<HttpRequest> request , std::shared_ptr<HttpResponse> response)
{
    // 1. 拼接请求资源
    // 2. 读取文件
    std::string path = base_dir_ + request->GetPath();
    if(path.back() == '/')
        path += "index.html";

    std::string body;
    bool ret = FileUtil::Read(path , body);
    if(!ret)
    {
        response->SetResponseCode(404);
        HandleError(response);
        return ;
    }
    response->SetBody(body , FileUtil::GetFileType(path));
    return ;
}

void HttpServer::Dispatcher(std::shared_ptr<HttpRequest> request , std::shared_ptr<HttpResponse> response ,std::vector<std::pair<std::regex , Handle> > route)
{
    // 进行分发
    std::string path = request->GetPath();
    for(auto& [e , handle] : route)
    {
        std::smatch matches;
        bool ret = std::regex_match(path , matches , e);
        if(!ret)
        {
            continue;
        }
        handle(request , response);
        return ;
    }

    // 没有资源
    response->SetResponseCode(404);
    HandleError(response);

    return ;
}


void HttpServer::Route(std::shared_ptr<HttpRequest> request , std::shared_ptr<HttpResponse> response)
{
    // 默认是页面请求 , 不是再根据请求方法进行路由
    if(IsFileRequest(request))
    {
        HandleFileRequest(request , response);
        return ;
    }

    std::string method = request->GetMethod();
    for(auto &ch :method)
    {
        ch = toupper(ch);
    }
    if(method == "GET")
    {
        Dispatcher(request , response , get_route_);
    }
    else if(method == "POST")
    {
        Dispatcher(request , response , post_route_);
    }
    else if(method == "PUT")
    {
        Dispatcher(request , response , put_route_);
    }
    else if(method == "DELETE")
    {
        Dispatcher(request , response , delete_route_);
    }
    else
    {
        response->SetResponseCode(405);
        HandleError(response);
    }

    return ;
}


void HttpServer::MesssageCallback(std::shared_ptr<Connection> conn , std::shared_ptr<Buffer> buf)
{
    // 1. 解析数据
    // 2. 获取 Request
    // 3. 根据 method 进行分发 , 默认是网页文件请求

    auto& context = std::any_cast<HttpContext&>(conn->GetContext());
    context.Parse(buf);

    auto parse_status = context.GetParseStatus();
    if(parse_status == ContextStatus::PARSELINE || parse_status == ContextStatus::PARSEHEADER || parse_status == ContextStatus::PARSEBODY)
    {
        return ;
    }

    auto response = std::make_shared<HttpResponse>();
    auto request = context.GetRequest();
    response->SetResponseCode(context.GetStatusCode());

    if(parse_status == ContextStatus::ERROR)
    {
        // 出现了错误直接进行返回
        HandleError(response);
        SendResponse(conn , response);
        context.Clear();
        return ;
    }
    if(request->KeepAlive())
    {
        response->SetKeepAlive();
    }

    // 正常处理 , 将方法进行路由
    Route(request , response);
    SendResponse(conn , response);
    context.Clear();
    return ;
}

HttpServer::HttpServer(uint16_t port)
:server_(port)
{
    server_.SetMessageCallback(std::bind(&HttpServer::MesssageCallback , this , std::placeholders::_1 , std::placeholders::_2));
    server_.SetNewConnectCallback(std::bind(&HttpServer::NewConnectCallback , this , std::placeholders::_1));
}

void HttpServer::NewConnectCallback(std::shared_ptr<Connection> conn )
{
    conn->SetContext(HttpContext());
}

void HttpServer::SetThreadCount(int count)
{
    server_.SetThreadCount(count);
}
void HttpServer::SetSelfRelease(size_t timeout)
{
    server_.SetSelfRelease(timeout);
}   

void HttpServer::Run()
{
    server_.Run();
}