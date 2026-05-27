
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

size_t HttpResponse::ContentLength()
{
    return body_.size();
}