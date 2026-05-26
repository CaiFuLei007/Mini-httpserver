
#include "base/util/string_util.h"

#include <sstream>
#include <cctype>
#include <iomanip>

int StringUtil::Spilt(const std::string &src , const std::string& sep , std::vector<std::string>& array)
{
    size_t cur = 0;
    while(cur < src.size())
    {
        size_t pos = src.find(sep , cur);
        if(pos == std::string::npos)
        {
            array.push_back(src.substr(cur));
            break;
        }
        if(pos != cur)
            array.push_back(src.substr(cur , pos - cur));
        cur = pos + sep.size();
    }
    return array.size();
}


std::string StringUtil::UrlEncode(const std::string& raw)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : raw) {
        if (std::isalnum(static_cast<unsigned char>(c)) || 
            c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } 
        else {
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
            escaped << std::nouppercase;
        }
    }
    return escaped.str();
}

std::string StringUtil::UrlDecode(const std::string& raw)
{
    auto hexToNum = [&](char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };

    std::string result;
    result.reserve(raw.length()); // 预分配内存提高效率

    for (size_t i = 0; i < raw.length(); ++i) {
        if (raw[i] == '%' && i + 2 < raw.length()) {
            char high = raw[i + 1];
            char low = raw[i + 2];
            
            if (std::isxdigit(high) && std::isxdigit(low)) {
                result += static_cast<char>((hexToNum(high) << 4) | hexToNum(low));
                i += 2; 
            } else {
                result += '%'; 
            }
        } else if (raw[i] == '+') {
            result += ' ';
        } else {
            result += raw[i];
        }
    }
    return result;
}