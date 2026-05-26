#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <string>
#include <vector>

#include "base/util/file_util.h"
#include "base/util/string_util.h"
#include "base/util/http_util.h"

// ==================== StringUtil::Spilt ====================

TEST(StringUtilSpiltTest, Normal) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("a,b,c", ",", result);
    ASSERT_EQ(cnt, 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST(StringUtilSpiltTest, EmptySrc) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("", ",", result);
    EXPECT_EQ(cnt, 0);
    EXPECT_TRUE(result.empty());
}

TEST(StringUtilSpiltTest, SepNotFound) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("hello", ",", result);
    ASSERT_EQ(cnt, 1);
    EXPECT_EQ(result[0], "hello");
}

// 实现会跳过连续分隔符之间的空 token
TEST(StringUtilSpiltTest, ConsecutiveSeps) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("a,,b", ",", result);
    ASSERT_EQ(cnt, 2);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
}

// 末尾分隔符同样不产生空 token
TEST(StringUtilSpiltTest, TrailingSep) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("a,b,", ",", result);
    ASSERT_EQ(cnt, 2);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
}

// 开头分隔符不产生空 token（前导 "/" 被跳过）
TEST(StringUtilSpiltTest, LeadingPathSep) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("/a/b/c", "/", result);
    ASSERT_EQ(cnt, 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST(StringUtilSpiltTest, MultiCharSep) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("hello::world::test", "::", result);
    ASSERT_EQ(cnt, 3);
    EXPECT_EQ(result[0], "hello");
    EXPECT_EQ(result[1], "world");
    EXPECT_EQ(result[2], "test");
}

TEST(StringUtilSpiltTest, SingleToken) {
    std::vector<std::string> result;
    int cnt = StringUtil::Spilt("only", "/", result);
    ASSERT_EQ(cnt, 1);
    EXPECT_EQ(result[0], "only");
}

// ==================== StringUtil::UrlEncode ====================

TEST(StringUtilUrlEncodeTest, Alphanumeric) {
    EXPECT_EQ(StringUtil::UrlEncode("abcABC123"), "abcABC123");
}

TEST(StringUtilUrlEncodeTest, SafeChars) {
    // RFC 3986 非编码字符：- _ . ~
    EXPECT_EQ(StringUtil::UrlEncode("-_.~"), "-_.~");
}

TEST(StringUtilUrlEncodeTest, Space) {
    EXPECT_EQ(StringUtil::UrlEncode(" "), "%20");
}

TEST(StringUtilUrlEncodeTest, Slash) {
    EXPECT_EQ(StringUtil::UrlEncode("/"), "%2F");
}

TEST(StringUtilUrlEncodeTest, QueryString) {
    EXPECT_EQ(StringUtil::UrlEncode("a=1&b=2"), "a%3D1%26b%3D2");
}

TEST(StringUtilUrlEncodeTest, EmptyString) {
    EXPECT_EQ(StringUtil::UrlEncode(""), "");
}

TEST(StringUtilUrlEncodeTest, UppercaseHex) {
    // 实现使用大写十六进制
    std::string encoded = StringUtil::UrlEncode(" ");
    EXPECT_EQ(encoded, "%20");
}

// ==================== StringUtil::UrlDecode ====================

TEST(StringUtilUrlDecodeTest, UppercaseHex) {
    EXPECT_EQ(StringUtil::UrlDecode("%2F"), "/");
}

TEST(StringUtilUrlDecodeTest, LowercaseHex) {
    EXPECT_EQ(StringUtil::UrlDecode("%2f"), "/");
}

TEST(StringUtilUrlDecodeTest, PlusToSpace) {
    EXPECT_EQ(StringUtil::UrlDecode("hello+world"), "hello world");
}

TEST(StringUtilUrlDecodeTest, PercentSpace) {
    EXPECT_EQ(StringUtil::UrlDecode("hello%20world"), "hello world");
}

TEST(StringUtilUrlDecodeTest, MultipleTokens) {
    EXPECT_EQ(StringUtil::UrlDecode("%61%62%63"), "abc");
}

TEST(StringUtilUrlDecodeTest, EmptyString) {
    EXPECT_EQ(StringUtil::UrlDecode(""), "");
}

// 末尾孤立的 % 原样保留
TEST(StringUtilUrlDecodeTest, TrailingPercent) {
    EXPECT_EQ(StringUtil::UrlDecode("abc%"), "abc%");
}

// %XY 中有非十六进制字符时，% 原样保留，后续字符继续独立处理
TEST(StringUtilUrlDecodeTest, InvalidHexChars) {
    EXPECT_EQ(StringUtil::UrlDecode("%GH"), "%GH");
}

TEST(StringUtilUrlDecodeTest, RoundTrip) {
    std::string original = "Hello World! foo=bar&baz=qux/path?key=val";
    EXPECT_EQ(StringUtil::UrlDecode(StringUtil::UrlEncode(original)), original);
}

// ==================== FileUtil Fixture ====================

class FileUtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = std::filesystem::temp_directory_path().string() + "/fileutil_gtest";
        std::filesystem::create_directories(test_dir_);
        test_file_ = test_dir_ + "/test.txt";
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }

    std::string test_dir_;
    std::string test_file_;
};

// ==================== FileUtil::Write / Read ====================

TEST_F(FileUtilTest, WriteAndRead) {
    std::string content = "Hello, FileUtil!";
    ASSERT_TRUE(FileUtil::Write(test_file_, content));

    std::string buf;
    ASSERT_TRUE(FileUtil::Read(test_file_, buf));
    EXPECT_EQ(buf, content);
}

TEST_F(FileUtilTest, WriteEmptyContent) {
    ASSERT_TRUE(FileUtil::Write(test_file_, ""));
    std::string buf;
    ASSERT_TRUE(FileUtil::Read(test_file_, buf));
    EXPECT_EQ(buf, "");
}

TEST_F(FileUtilTest, WriteMultiLine) {
    std::string content = "line1\nline2\nline3\n";
    ASSERT_TRUE(FileUtil::Write(test_file_, content));
    std::string buf;
    ASSERT_TRUE(FileUtil::Read(test_file_, buf));
    EXPECT_EQ(buf, content);
}

TEST_F(FileUtilTest, WriteOverwrite) {
    ASSERT_TRUE(FileUtil::Write(test_file_, "old content"));
    ASSERT_TRUE(FileUtil::Write(test_file_, "new content"));
    std::string buf;
    ASSERT_TRUE(FileUtil::Read(test_file_, buf));
    EXPECT_EQ(buf, "new content");
}

// 写入不存在的中间目录应失败
TEST_F(FileUtilTest, WriteInvalidPath) {
    EXPECT_FALSE(FileUtil::Write(test_dir_ + "/no_such_subdir/file.txt", "data"));
}

TEST_F(FileUtilTest, ReadNonExisting) {
    std::string buf;
    EXPECT_FALSE(FileUtil::Read(test_dir_ + "/ghost.txt", buf));
}

// ==================== FileUtil::GetFileType ====================

TEST(FileUtilGetTypeTest, Html) {
    EXPECT_EQ(FileUtil::GetFileType("index.html"), "text/html");
}

TEST(FileUtilGetTypeTest, Htm) {
    EXPECT_EQ(FileUtil::GetFileType("page.htm"), "text/html");
}

TEST(FileUtilGetTypeTest, Jpg) {
    EXPECT_EQ(FileUtil::GetFileType("photo.jpg"), "image/jpeg");
}

TEST(FileUtilGetTypeTest, Jpeg) {
    EXPECT_EQ(FileUtil::GetFileType("photo.jpeg"), "image/jpeg");
}

TEST(FileUtilGetTypeTest, Png) {
    EXPECT_EQ(FileUtil::GetFileType("image.png"), "image/png");
}

TEST(FileUtilGetTypeTest, Css) {
    EXPECT_EQ(FileUtil::GetFileType("style.css"), "text/css");
}

TEST(FileUtilGetTypeTest, Js) {
    EXPECT_EQ(FileUtil::GetFileType("app.js"), "text/javascript");
}

TEST(FileUtilGetTypeTest, Json) {
    EXPECT_EQ(FileUtil::GetFileType("data.json"), "application/json");
}

TEST(FileUtilGetTypeTest, Txt) {
    EXPECT_EQ(FileUtil::GetFileType("readme.txt"), "text/plain");
}

TEST(FileUtilGetTypeTest, Pdf) {
    EXPECT_EQ(FileUtil::GetFileType("document.pdf"), "application/pdf");
}

TEST(FileUtilGetTypeTest, Mp3) {
    EXPECT_EQ(FileUtil::GetFileType("song.mp3"), "audio/mpeg");
}

TEST(FileUtilGetTypeTest, Svg) {
    EXPECT_EQ(FileUtil::GetFileType("icon.svg"), "image/svg+xml");
}

TEST(FileUtilGetTypeTest, Zip) {
    EXPECT_EQ(FileUtil::GetFileType("archive.zip"), "application/zip");
}

TEST(FileUtilGetTypeTest, NoExtension) {
    EXPECT_EQ(FileUtil::GetFileType("Makefile"), "application/octet-stream");
}

TEST(FileUtilGetTypeTest, UnknownExtension) {
    EXPECT_EQ(FileUtil::GetFileType("file.xyz"), "application/octet-stream");
}

TEST(FileUtilGetTypeTest, WithAbsolutePath) {
    EXPECT_EQ(FileUtil::GetFileType("/var/www/html/index.html"), "text/html");
}

TEST(FileUtilGetTypeTest, DotInDirName) {
    // rfind('.') 找到最后一个点，确保目录名中的点不干扰
    EXPECT_EQ(FileUtil::GetFileType("/my.dir/style.css"), "text/css");
}

// ==================== FileUtil::IsRegularFile ====================

TEST_F(FileUtilTest, IsRegularFile_File) {
    FileUtil::Write(test_file_, "data");
    EXPECT_TRUE(FileUtil::IsRegularFile(test_file_));
}

TEST_F(FileUtilTest, IsRegularFile_Directory) {
    EXPECT_FALSE(FileUtil::IsRegularFile(test_dir_));
}

TEST_F(FileUtilTest, IsRegularFile_NonExisting) {
    EXPECT_FALSE(FileUtil::IsRegularFile(test_dir_ + "/ghost.txt"));
}

// ==================== FileUtil::IsDirectory ====================

TEST_F(FileUtilTest, IsDirectory_Directory) {
    EXPECT_TRUE(FileUtil::IsDirectory(test_dir_));
}

TEST_F(FileUtilTest, IsDirectory_File) {
    FileUtil::Write(test_file_, "data");
    EXPECT_FALSE(FileUtil::IsDirectory(test_file_));
}

TEST_F(FileUtilTest, IsDirectory_NonExisting) {
    EXPECT_FALSE(FileUtil::IsDirectory(test_dir_ + "/ghost_dir"));
}

// ==================== FileUtil::IsValidPath ====================

TEST(FileUtilPathTest, SimplePath) {
    EXPECT_TRUE(FileUtil::IsValidPath("a/b/c"));
}

TEST(FileUtilPathTest, SingleSegment) {
    EXPECT_TRUE(FileUtil::IsValidPath("a"));
}

TEST(FileUtilPathTest, EmptyPath) {
    EXPECT_TRUE(FileUtil::IsValidPath(""));
}

TEST(FileUtilPathTest, DotSegment) {
    EXPECT_TRUE(FileUtil::IsValidPath("a/./b"));
}

// a/b/../c → level: 1,2,1,2 合法
TEST(FileUtilPathTest, DotDotValid) {
    EXPECT_TRUE(FileUtil::IsValidPath("a/b/../c"));
}

// ../etc → level 立即变 -1，非法（路径穿越）
TEST(FileUtilPathTest, DotDotAtRoot) {
    EXPECT_FALSE(FileUtil::IsValidPath("../etc/passwd"));
}

// a/../../b → level: 1,0,-1，非法
TEST(FileUtilPathTest, DotDotBeyondRoot) {
    EXPECT_FALSE(FileUtil::IsValidPath("a/../../b"));
}

// 绝对路径：前导 "/" 对应空 token 被跳过，视为合法
TEST(FileUtilPathTest, AbsolutePath) {
    EXPECT_TRUE(FileUtil::IsValidPath("/a/b/c"));
}

// ==================== HttpUtil::GetResponseCodeDes ====================
//
// BUG: http_util.h 中 GetResponseCodeDes 被声明在 class 的默认 private 区域，
//      外部无法调用。需将其改为 public，否则以下测试无法编译。
//
// BUG: http_util.cc 中 500 (Internal Server Error) 缺失，下面测试可验证此问题。

TEST(HttpUtilTest, Code200) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(200), "OK");
}

TEST(HttpUtilTest, Code201) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(201), "Created");
}

TEST(HttpUtilTest, Code204) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(204), "No Content");
}

TEST(HttpUtilTest, Code301) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(301), "Moved Permanently");
}

TEST(HttpUtilTest, Code302) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(302), "Found");
}

TEST(HttpUtilTest, Code304) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(304), "Not Modified");
}

TEST(HttpUtilTest, Code400) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(400), "Bad Request");
}

TEST(HttpUtilTest, Code401) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(401), "Unauthorized");
}

TEST(HttpUtilTest, Code403) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(403), "Forbidden");
}

TEST(HttpUtilTest, Code404) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(404), "Not Found");
}

TEST(HttpUtilTest, Code405) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(405), "Method Not Allowed");
}

TEST(HttpUtilTest, Code500) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(500), "Internal Server Error");
}

TEST(HttpUtilTest, Code502) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(502), "Bad Gateway");
}

TEST(HttpUtilTest, Code503) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(503), "Service Unavailable");
}

TEST(HttpUtilTest, Code100) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(100), "Continue");
}

TEST(HttpUtilTest, Code101) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(101), "Switching Protocol");
}

TEST(HttpUtilTest, Code418) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(418), "I'm a teapot");
}

TEST(HttpUtilTest, UnknownCode) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(999), "Unknow");
}

TEST(HttpUtilTest, NegativeCode) {
    EXPECT_EQ(HttpUtil::GetResponseCodeDes(-1), "Unknow");
}
