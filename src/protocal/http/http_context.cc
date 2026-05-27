
#include "protocal/http/http_context.h"
#include "base/util/string_util.h"

#include <regex>

bool HttpContext::ParseLine(const std::string& line)
{
    // 解析请求头
    std::smatch matches;
    std::regex e("(GET|HEAD|POST|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?", std::regex::icase);
    bool ret = std::regex_match(line , matches , e);
    if(!ret)
    {
        status_code_ = 400 ;
        is_error_ = true;
        parse_stats_ = ContextStatus::ERROR;
        return false;
    }
    request_->SetMethod(matches[1]);
    request_->SetPath(matches[2]);
    request_->SetVersion(matches[4]);

    std::string url = matches[3];
    // 解析 url 参数 , 从 & 进行分割 , 再从 = 进行分割
    std::vector<std::string> params;
    StringUtil::Spilt(url, "&" , params);
    for(auto& param : params)
    {
        std::vector<std::string> tmp;
        StringUtil::Spilt(param , "=" , tmp);
        if(tmp.size() != 2)
        {
            continue;
        }
        request_->SetParam(StringUtil::UrlDecode(tmp[0]) , StringUtil::UrlDecode(tmp[1]));
    }
    parse_stats_ = ContextStatus::PARSEHEADER;
    return true;
}

bool HttpContext::HandleLine(std::shared_ptr<Buffer> buf)
{
    static int kLineMaxSize = 1024;

    if(parse_stats_ != ContextStatus::PARSELINE)
    {
        return false;
    }
    // 获取一行 , 进行解析
    std::string line = buf->GetLineAndPop();
    while(line.size() && (line.back() == '\r' || line.back() == '\n'))
    {
        line.pop_back();
    }
    if(line.empty())
    {
        // line 为空
        if(buf->Size() > kLineMaxSize)
        {
            // 请求行太长 , 错误
            status_code_ = 414 ;
            is_error_ = true;
            parse_stats_ = ContextStatus::ERROR;
            return false;
        }
        return true;
    }
    
    return ParseLine(line);
}

bool HttpContext::ParseHeader(const std::string& data)
{
    std::vector<std::string> tmp ;
    StringUtil::Spilt(data , ": " , tmp);
    if(tmp.size() != 2)
    {
        return false;
    }
    if(tmp[0] == "Connection" && tmp[1] == "keep-alive")
    {
        request_->SetKeepAlive();
    }

    request_->SetHeader(tmp[0] , tmp[1]);
    return true;
}

bool HttpContext::HandleHeader(std::shared_ptr<Buffer> buf)
{
    if(parse_stats_ != ContextStatus::PARSEHEADER)
    {
        return false;
    }
    while(buf->Size() > 0)
    {
        std::string date = buf->GetLineAndPop();
        if(date.empty())
        {
            return true;
        }
        while(date.size() && (date.back() == '\n' || date.back() == '\r'))
        {
            date.pop_back();
        }
        if(date.empty())   // \n\n
        {
            parse_stats_ = ContextStatus::PARSEBODY;
            return true;
        }
        ParseHeader(date);
    }
    return true;
}
 
bool HttpContext::ParseBody(const std::string& data)
{
    request_->AppendBody(data);
    return true;
}

bool HttpContext::HandleBody(std::shared_ptr<Buffer> buf)
{
    if(parse_stats_ != ContextStatus::PARSEBODY)
    {
        return false;
    }
    size_t need_size = request_->ContentLength() - request_->BodyLength();
    if(buf->Size() >= need_size)
    {
        std::string data;
        data.resize(need_size);
        buf->ReadAndPop(&data[0] , need_size);
        ParseBody(data);

        parse_stats_ = ContextStatus::FINISH;
        return true;
    }
    std::string data ;
    buf->ReadAndPop(data);
    ParseBody(data);
    return true;
}

void HttpContext::Parse(std::shared_ptr<Buffer> buf)
{
    switch(parse_stats_)
    {
        case ContextStatus::PARSELINE:
            HandleLine(buf);
        case ContextStatus::PARSEHEADER:
            HandleHeader(buf);
        case ContextStatus::PARSEBODY:
            HandleBody(buf);  
    }
}