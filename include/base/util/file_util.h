
#pragma once

/*
    - 文件工具类
    - 接口 :
            1) 读取
            2) 写入
            3) 获取文件类型
            4) 判断文件是不是目录  , 是不是普通文件
            5) 判断路径是否是有效路径

*/

#include <string>

class FileUtil
{
public:
    static bool Read(const std::string &path , std::string& buf);
    static bool Write(const std::string &path , const std::string &buf);

    static std::string GetFileType(const std::string &file);

    static bool IsDirectory(const std::string &path);
    static bool IsRegularFile(const std::string &path);

    static bool IsValidPath(const std::string& path);
};