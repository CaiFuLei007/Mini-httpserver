# =================================================================
#                         基础配置
# =================================================================

cmake_minimum_required(VERSION 3.18)

project(
    MyProject
    VERSION 1.0.0
    LANGUAGES CXX
)

# 设置C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

 # 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

# 设置可执行程序构建类型
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

# 设置优化级别
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")


# =================================================================
#                         生成可执行程序
# =================================================================

file(GLOB SRC_LISTS 
    "src/*.cc"
    "src/base/*.cc"
    "src/base/util/*.cc"
    "src/server/*.cc"
    "Test/*.hpp"
)

add_executable(all_test
    Test/all_test.cc
    ${SRC_LISTS}
)
# add_executable(echo_server
#     Test/echo_server.cc
#     ${SRC_LISTS}
# )

# =================================================================
#                         添加依赖
# =================================================================


include_directories(
    ${CMAKE_SOURCE_DIR}/external/quill_lib/include
    ${CMAKE_SOURCE_DIR}/include
)

find_package(GTest REQUIRED)

target_link_libraries(
    all_test
    PRIVATE
    GTest::gtest GTest::gtest_main
)
