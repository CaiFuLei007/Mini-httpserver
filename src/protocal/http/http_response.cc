
#include "protocal/http/http_response.h"

std::string HttpResponse::GetParam(const std::string &key)
{
    auto it = params_.find(key);
    if(it == params_.end())
    {
        return "";
    }
    return it->second;
}
std::string HttpResponse::GetHeader(const std::string& key)
{
    auto it = headers_.find(key);
    if(it == headers_.end())
    {
        return "";
    }
    return it->second;
}

size_t HttpResponse::BodyLength()
{
    auto it = headers_.find("Content-Length");
    if(it == headers_.end())
    {
        return 0;
    }
    return std::stoul(it->second);
}