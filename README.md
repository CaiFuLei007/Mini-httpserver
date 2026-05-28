# Mini-httpserver - 轻量级 C++ HTTP 服务器

**作者**：CaiFuLei007  
**开发环境**：Ubuntu VS Code  
**编译器**：g++  
**编程语言**：C++20  
**构建工具**：CMake 3.18+

## 目录

- [Mini-httpserver - 轻量级 C++ HTTP 服务器](#mini-httpserver---轻量级-c-http-服务器)
  - [目录](#目录)
  - [项目简介](#项目简介)
  - [项目结构](#项目结构)
  - [代码统计](#代码统计)
  - [项目编译](#项目编译)
  - [测试环境](#测试环境)
  - [项目测试](#项目测试)
    - [单元测试](#单元测试)
    - [Echo 服务器测试](#echo-服务器测试)
    - [HTTP 服务器测试](#http-服务器测试)
  - [压测结果](#压测结果)
    - [压测工具：wrk](#压测工具wrk)
    - [localhost 单机压测（WARN 日志级别，ThreadCount=3）](#localhost-单机压测warn-日志级别threadcount3)
    - [公网跨机器压测（本地 → 云服务器）](#公网跨机器压测本地--云服务器)
    - [日志级别对比](#日志级别对比)
    - [内存分析](#内存分析)
  - [已知缺陷](#已知缺陷)
  - [总结](#总结)
    - [关键设计特点](#关键设计特点)
  - [致谢](#致谢)
  - [支持项目](#支持项目)


## 项目简介

基于 C++20 开发的轻量级 HTTP 服务器，参考陈硕 [muduo](https://github.com/chenshuo/muduo) 网络库的 Reactor 架构，使用 C++ 标准库替代 Boost 依赖，集成 [quill](https://github.com/odygrd/quill) 异步日志库。支持 GET/POST/PUT/DELETE 路由、静态文件服务、HTTP keep-alive 长连接，单机 QPS 可达 6 万。

## 项目结构

```
Mini-httpserver/
├── CMakeLists.txt              # CMake 构建配置
├── external/                   # 第三方库（quill header-only）
├── web/                        # 静态文件根目录
│   ├── index.html
│   └── 404.html
├── include/                    # 头文件
│   ├── base/                   # 基础组件
│   │   ├── buffer.h            #   Buffer 用户态缓冲区
│   │   ├── channel.h           #   Channel fd 事件管理
│   │   ├── poller.h            #   Poller epoll 封装
│   │   ├── socket.h            #   Socket RAII 封装
│   │   └── util/               #   工具类
│   │       ├── file_util.h     #     文件读写、MIME 识别
│   │       ├── http_util.h     #     HTTP 状态码映射
│   │       ├── quill_log.h     #     quill 日志封装
│   │       └── string_util.h   #     字符串分割、URL 编解码
│   ├── server/                 # 服务端核心
│   │   ├── acceptor.h          #   Acceptor 监听器
│   │   ├── eventloop.h         #   EventLoop 事件循环
│   │   ├── loopthread_poll.h   #   LoopThread / LoopThreadPool
│   │   └── timerwheel.h        #   TimerWheel 时间轮
│   ├── protocal/http/          # HTTP 协议
│   │   ├── http_context.h      #   HttpContext 增量解析状态机
│   │   ├── http_resquest.h     #   HttpRequest 请求体
│   │   └── http_response.h     #   HttpResponse 响应体
│   ├── connection.h            # Connection 连接管理
│   ├── http_server.h           # HttpServer HTTP 服务器
│   └── tcp_server.h            # TcpServer 服务器整合
├── src/                        # 源文件（与 include 头文件一一对应）
│   ├── base/
│   ├── server/
│   ├── protocal/http/
│   ├── connection.cc
│   ├── http_server.cc
│   └── tcp_server.cc
└── Test/                       # 测试与示例
    ├── all_test.cc             #   单元测试入口
    ├── *_test.hpp              #   各模块单元测试
    ├── echo_server.cc          #   Echo 服务示例
    └── http_server_test.cc     #   HTTP 服务示例
```

## 代码统计

| 项目 | 数值 |
|------|------|
| 总行数 | 6,598 |
| 核心库 (src + include) | 3,332 行 |
| 单元测试 | 3,266 行（部分测试用例由 AI 辅助编写） |
| 源文件数 | 46（含测试） |
| 测试覆盖模块 | Buffer, Socket, Poller, TimerWheel, EventLoop, Connection, Utils |

## 项目编译

项目基于 CMake，拉取后编译即可：

```bash
git clone <repo-url>
cd Mini-httpserver

# 编译（Debug 模式默认开启）
mkdir build && cd build
cmake ..
make -j$(nproc)

# 运行示例（从 build 目录）
./bin/release/echo_server              # Echo 服务，端口 8888
./bin/release/http_server_test ../web  # HTTP 服务，端口 8080
./bin/release/all_test                 # 单元测试
```

编译依赖：
- CMake 3.18+
- g++ 支持 C++20
- Google Test（`libgtest-dev`）
- quill 库已内嵌在 `external/quill_lib/` 中，无需额外安装

```bash
# Ubuntu 安装 GTest
sudo apt install libgtest-dev
```

## 测试环境

| 项目 | 规格 |
|------|------|
| CPU | Intel Xeon Platinum (2C4T) @ 2.5GHz |
| 内存 | 1.6 GB |
| 磁盘 | 40 GB SSD |
| 操作系统 | Ubuntu 22.04 LTS |
| 内核 | Linux 5.15.0-142-generic |
| 编译器 | g++ 11.4.0 |
| 构建工具 | CMake 3.22.1 |
| 第三方库 | quill（异步日志）、Google Test 1.16.0 |

## 项目测试

### 单元测试

使用 Google Test 框架，覆盖以下模块：

| 测试文件 | 覆盖模块 | 测试点数 |
|----------|----------|----------|
| `buffer_test.hpp` | Buffer 读写、扩容、行提取 | 27 |
| `socket_test.hpp` | Socket 创建、绑定、收发 | ~20 |
| `poller_test.hpp` | Poller 事件注册/更新/移除 | ~10 |
| `timerwheel_test.hpp` | TimerWheel 添加/更新/取消 | ~15 |
| `eventloop_test.hpp` | EventLoop 任务队列、定时器 | ~30 |
| `connection_test.hpp` | Connection 生命周期、收发 | ~35 |
| `util_test.hpp` | StringUtil、FileUtil、HttpUtil | 42 |

```bash
cd build
./bin/all_test
```

### Echo 服务器测试

`Test/echo_server.cc` — 一个简单的回显服务器，监听 8888 端口：

```cpp
TcpServer server(8888);

server.SetMessageCallback([](shared_ptr<Connection> conn, shared_ptr<Buffer> buf) {
    string msg;
    buf->ReadAndPop(msg);
    conn->SendMessage(msg);
});

server.SetThreadCount(3);
server.SetSelfRelease(10);
server.Run();
```

```bash
# 启动
./bin/echo_server
# 另开终端测试
telnet localhost 8888
```

### HTTP 服务器测试

`Test/http_server_test.cc` — 支持静态文件 + 路由的 HTTP 服务：

```bash
./bin/release/http_server_test ../web
# 访问 http://localhost:8080/hello
# 访问 http://localhost:8080/user?name=张三
# curl -X POST -d "username=admin&password=123" http://localhost:8080/login
# curl -X PUT -d "data=hello" http://localhost:8080/update
# curl -X DELETE http://localhost:8080/remove
```

预置路由：
| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/hello` | 返回 JSON `{"message": "Hello, World!"}` |
| GET | `/user?name=xxx` | 返回 JSON `{"user": "xxx"}` |
| POST | `/login` | 回显 POST body |
| POST | `/echo` | 回显 POST body（纯文本） |
| PUT | `/update` | 返回更新确认 |
| DELETE | `/remove` | 返回删除确认 |
| GET | `/` | 静态文件 `web/index.html`（3D 电竞房场景） |

## 压测结果

### 压测工具：wrk

```bash
# 安装
git clone https://github.com/wg/wrk.git
cd wrk && make -j$(nproc) && sudo cp wrk /usr/local/bin/

# 压测
wrk -t4 -c1000 -d30s --latency http://localhost:8080/hello
```

**目标接口**：`GET /hello`，响应体 `{"message": "Hello, World!"}`（28 bytes）

### localhost 单机压测（WARN 日志级别，ThreadCount=3）

| 场景 | 工具 | QPS | P50 | P99 | 带宽 |
|------|------|-----|-----|-----|------|
| c1000 keep-alive | wrk -t4 | **60,827** | 16.24ms | 29.69ms | 7.14 MB/s |
| c5000 keep-alive | wrk -t4 | 60,087 | 82.32ms | 122.49ms | 7.05 MB/s |
| c10000 keep-alive | wrk -t4 | 56,472 | 165.59ms | 263.04ms | 6.62 MB/s |
| c15000 keep-alive | wrk -t4 | 49,879 | 293.39ms | 460.67ms | 5.85 MB/s |
| c20000 keep-alive | wrk -t4 | 45,566 | 419.34ms | 604.47ms | 5.35 MB/s |
| c25000 keep-alive | wrk -t4 | 43,881 | 552.49ms | 805.41ms | 5.15 MB/s |
| c30000 keep-alive | wrk -t4 | 41,561 | 702.16ms | 913.59ms | 4.88 MB/s |

### 公网跨机器压测（本地 → 云服务器）

| 场景 | 工具 | QPS | P50 | P99 | 带宽 | 说明 |
|------|------|-----|-----|-----|------|------|
| c5000 keep-alive | wrk -t4 | 5,109 | 306.56ms | 1.85s | 614 KB/s | 7,787 timeout，公网带宽瓶颈 |

**关键结论：**
- QPS 峰值 **6 万**（c1000），连接数从 1000 到 3 万 QPS 从 6 万逐步降至 4.2 万，仅降 31%
- P50 延迟与连接数线性相关：c1000 仅 16ms，c30000 达 702ms
- 公网跨机器 QPS 仅 5,109，受限于公网带宽和延迟，非服务端瓶颈

**关键结论：**
- QPS 极其稳定，从 500 到 3 万连接始终维持在 **3.7-4.3 万**，吞吐几乎不受连接数影响
- P50 延迟随连接数线性增长：c500 仅 13ms，c30000 达 664ms，每增加 5000 连接约增加 100ms
- P99 与 P50 差距小（c30000 仅差 193ms），延迟抖动低，服务端处理均匀稳定
- 3 万 keep-alive 长连接下服务端依然正常工作，无 segfault，无 fd 泄漏
- 单连接 RSS ~1.5KB（Connection + 2×1024 Buffer + 内核 socket 缓冲区映射），3 万连接 ≈ 45MB

### 日志级别对比

| 场景 | 工具 | QPS | 带宽 | 说明 |
|------|------|-----|------|------|
| c100 WARN | wrk -t4 | **47,235** | 5.54 MB/s | 短路 DEBUG/INFO 日志 |
| c100 DEBUG | wrk -t4 | 41,296 | 4.84 MB/s | QPS 降 **12.6%**，quill 异步日志开销有限 |

### 内存分析

- **空载 RSS**：3.8 MB（含 4 个 EventLoop 线程栈 + quill 后端线程）
- **单连接内存开销**：~1.5KB（Connection + 收发 Buffer 各 1024 预分配 + 内核 socket 缓冲区）
- **高并发预测**：1 万长连接 ≈ **19MB**；5 万长连接 ≈ **79MB**

---

## 已知缺陷

- **单 Acceptor 瓶颈**：单 listener 在极端短连接场景（>5 万 accept/s）可能成为瓶颈，多核下可改用 `SO_REUSEPORT` + 多 Acceptor
- **`_next_id` 共用计数器**：连接 ID 和定时任务 ID 共用 `TcpServer::next_id_`，语义不清，后续扩展时容易踩坑
- **时间轮精度粗**：1s tick，不适用于毫秒级超时
- **HTTP 解析不完整**：不支持 chunked 编码、Trailer 头、`Expect: 100-continue`、多部分表单上传
- **无背压机制**：输出缓冲区无上限，慢客户端可撑爆服务端内存
- **无安全机制**：无 TLS、无连接数限制、无请求速率限制



## 总结

### 关键设计特点

- **one thread one loop**：每个 I/O 线程一个 EventLoop，无锁竞争
- **跨线程唤醒**：通过 eventfd 实现线程安全的任务提交
- **缓冲区设计**：用户态 Buffer 减少系统调用，读/写分离
- **时间轮定时器**：O(1) 复杂度管理连接超时
- **RAII 贯穿始终**：Socket/Channel/EventLoop 等资源在析构时自动释放
- **`std::any` 上下文**：实现协议无关的 Connection，通过 `context_` 注入 HttpContext

## 致谢

- 感谢陈硕的 [muduo](https://github.com/chenshuo/muduo) 网络库提供的设计灵感
- 感谢 [quill](https://github.com/odygrd/quill) 提供的异步日志库
- 感谢 Google Test 提供的单元测试框架

## 支持项目

如果这个项目对你有帮助，请给它一个 Star

发现问题？欢迎提交 Issue

有改进建议？欢迎提交 Pull Request
