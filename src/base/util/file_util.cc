
#include "base/util/file_util.h"
#include "base/util/string_util.h"

#include <fstream>
#include <unordered_map>
#include <filesystem>

bool FileUtil::Read(const std::string &path, std::string &buf)
{
    std::ifstream ifs(path);
    if (!ifs)
    {
        return false;
    }
    size_t sz = 0;
    ifs.seekg(0, std::ios::end);
    sz = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    buf.resize(sz);
    if (sz == 0)
    {
        return true;
    }
    ifs.read(&buf[0], sz);
    if (!ifs.good())
    {
        return false;
    }
    return true;
}
bool FileUtil::Write(const std::string &path, const std::string &buf)
{
    std::ofstream ofs(path);
    if (!ofs)
    {
        return false;
    }
    ofs.write(buf.c_str(), buf.size());
    if (!ofs.good())
    {
        return false;
    }
    return true;
}

std::string FileUtil::GetFileType(const std::string &file)
{
    static std::unordered_map<std::string, std::string> mime_msg = {
        {".aac", "audio/aac"},
        {".abw", "application/x-abiword"},
        {".arc", "application/x-freearc"},
        {".avi", "video/x-msvideo"},
        {".azw", "application/vnd.amazon.ebook"},
        {".bin", "application/octet-stream"},
        {".bmp", "image/bmp"},
        {".bz", "application/x-bzip"},
        {".bz2", "application/x-bzip2"},
        {".csh", "application/x-csh"},
        {".css", "text/css"},
        {".csv", "text/csv"},
        {".doc", "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {".eot", "application/vnd.ms-fontobject"},
        {".epub", "application/epub+zip"},
        {".gif", "image/gif"},
        {".htm", "text/html"},
        {".html", "text/html"},
        {".ico", "image/vnd.microsoft.icon"},
        {".ics", "text/calendar"},
        {".jar", "application/java-archive"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".js", "text/javascript"},
        {".json", "application/json"},
        {".jsonld", "application/ld+json"},
        {".mid", "audio/midi"},
        {".midi", "audio/x-midi"},
        {".mjs", "text/javascript"},
        {".mp3", "audio/mpeg"},
        {".mpeg", "video/mpeg"},
        {".mpkg", "application/vnd.apple.installer+xml"},
        {".odp", "application/vnd.oasis.opendocument.presentation"},
        {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
        {".odt", "application/vnd.oasis.opendocument.text"},
        {".oga", "audio/ogg"},
        {".ogv", "video/ogg"},
        {".ogx", "application/ogg"},
        {".otf", "font/otf"},
        {".png", "image/png"},
        {".pdf", "application/pdf"},
        {".ppt", "application/vnd.ms-powerpoint"},
        {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        {".rar", "application/x-rar-compressed"},
        {".rtf", "application/rtf"},
        {".sh", "application/x-sh"},
        {".svg", "image/svg+xml"},
        {".swf", "application/x-shockwave-flash"},
        {".tar", "application/x-tar"},
        {".tif", "image/tiff"},
        {".tiff", "image/tiff"},
        {".ttf", "font/ttf"},
        {".txt", "text/plain"},
        {".vsd", "application/vnd.visio"},
        {".wav", "audio/wav"},
        {".weba", "audio/webm"},
        {".webm", "video/webm"},
        {".webp", "image/webp"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".xhtml", "application/xhtml+xml"},
        {".xls", "application/vnd.ms-excel"},
        {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {".xml", "application/xml"},
        {".xul", "application/vnd.mozilla.xul+xml"},
        {".zip", "application/zip"},
        {".3gp", "video/3gpp"},
        {".3g2", "video/3gpp2"},
        {".7z", "application/x-7z-compressed"}
    };

    size_t pos = file.rfind('.');
    if(pos == std::string::npos)
    {
        return "application/octet-stream" ;   // 二进制流
    }
    std::string extension = file.substr(pos);
    auto it = mime_msg.find(extension);
    if(it == mime_msg.end())
    {
        return "application/octet-stream" ;   // 二进制流
    }
    return it->second;
}



bool FileUtil::IsDirectory(const std::string &path)
{
    return std::filesystem::is_directory(path);
}
bool FileUtil::IsRegularFile(const std::string &path)
{
    return std::filesystem::is_regular_file(path);
}

bool FileUtil::IsValidPath(const std::string& path)
{
    std::vector<std::string> nodes;
    StringUtil::Spilt(path , "/" , nodes);
    int level = 0;
    for(auto& node : nodes)
    {
        if(node == "..")
        {
            level--;
        }
        else if(node != ".")
        {
            level++;
        }

        if(level < 0)
        {
            return false;
        }
    }
    return true;
}