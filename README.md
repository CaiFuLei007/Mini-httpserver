# MiniMuduo - 轻量级 C++ HTTP 服务器

**作者**：CaiFuLei007  
**开发环境**：Ubuntu VS Code  
**编译器**：g++  
**编程语言**：C++20  
**构建工具**：CMake 3.18+

## 测试环境

| 项目 | 规格 |
|------|------|
| CPU | AMD Ryzen 7 6800H (8C8T) @ 3.2GHz |
| 内存 | 8 GB DDR5 |
| 操作系统 | Ubuntu 22.04.5 LTS |
| 内核 | Linux 6.8.0-111-generic |
| 编译器 | g++ 11.4.0 |
| 构建工具 | CMake 3.22.1 |
| 第三方库 | quill（异步日志）、Google Test 1.16.0 |

## 如何复现

```bash
git clone <repo-url>
cd Mini-httpserver
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./bin/release/echo_server       # Echo 服务（端口 8888）
./bin/release/http_server_test  # HTTP 服务（端口 8080）
./bin/release/all_test          # 单元测试
```

## 代码统计

| 项目 | 数值 |
|------|------|
| 总行数 | 6,598 |
| 核心库 (src + include) | 3,332 行 |
| 单元测试 | 3,266 行（部分测试用例由 AI 辅助编写） |
| 源文件数 | 46（含测试） |
| 测试覆盖模块 | Buffer, Socket, Poller, TimerWheel, EventLoop, Connection, Utils |

## 已知缺陷


- **单 Acceptor 瓶颈**：单 listener 在极端短连接场景（>5 万 accept/s）可能成为瓶颈，多核下可改用 `SO_REUSEPORT` + 多 Acceptor
- **`_next_id` 共用计数器**：连接 ID 和定时任务 ID 共用 `TcpServer::next_id_`，使用起来并没有影响，但语义不清，后续扩展时容易踩坑
- **时间轮精度粗**：1s tick，不适用于毫秒级超时
- **HTTP 解析不完整**：不支持 chunked 编码、Trailer 头、`Expect: 100-continue`、多部分表单上传
- **无背压机制**：输出缓冲区无上限，慢客户端可撑爆服务端内存
- **无安全机制**：无 TLS、无连接数限制、无请求速率限制


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
./bin/release/echo_server         # Echo 服务，端口 8888
./bin/release/http_server_test    # HTTP 服务，端口 8080
./bin/release/all_test            # 单元测试
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
./bin/http_server_test
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
wrk -t4 -c100 -d30s --latency http://localhost:8080/hello
```

**测试环境**：AMD Ryzen 7 6800H (8C8T)、8GB DDR5、Ubuntu 22.04、g++ 11.4.0 Release

**目标接口**：`GET /hello`，响应体 `{"message": "Hello, World!"}`（28 bytes）

| 并发数 | 持续时间 | QPS | 平均延迟 | P50 | P99 | 总请求 | Socket 错误 |
|--------|----------|-----|----------|-----|-----|--------|-------------|
| 100 | 30s | **44,649** | 2.23ms | 2.21ms | 4.25ms | 1,340,507 | **0** |
| 500 | 30s | 43,320 | 11.46ms | 11.20ms | 20.83ms | 1,303,556 | **0** |
| 1,000 | 30s | 41,703 | 23.85ms | 23.81ms | 28.92ms | 1,252,185 | **0** |
| 2,000 | 30s | 35,480 | 56.03ms | 56.08ms | 66.31ms | 1,066,650 | **0** |


---

## 项目概述

这是一个基于 C++20 开发的轻量级 HTTP 服务器，参考陈硕 [muduo](https://github.com/chenshuo/muduo) 网络库的 Reactor 架构设计，使用 C++11/17/20 标准库替代 Boost 依赖，集成了 [quill](https://github.com/odygrd/quill) 异步日志库。

核心目标：
- 学习 Reactor 网络模型和事件驱动编程
- 掌握 Linux epoll、非阻塞 I/O、多线程编程
- 理解 HTTP 协议的解析与路由分派
- 实践 C++ 现代特性（智能指针、`std::function`、`std::any`、RAII）

## Reactor 模型

本项目采用 **主从 Reactor（Main-Sub Reactor）** 模型：

- **Reactor**：即非阻塞同步 I/O 模型——应用程序向内核注册感兴趣的事件（可读、可写），内核在事件就绪时通知应用程序，应用程序自己执行实际的 I/O 操作
- **Proactor**：异步 I/O 模型——应用程序发起 I/O 操作时不仅注册事件还提供缓冲区，内核完成整个 I/O 操作后再通知应用程序

### Reactor 模型核心组件

```
Event          → 事件（文件描述符上的可读/可写/错误）
Reactor        → 反应堆（事件循环，驱动整个流程）
Demultiplex    → 多路事件分发器（epoll_wait）
EventHandler   → 事件处理器（各个回调函数）
```

### 调用流程

1. EventHandler 注册 Event 到 Reactor
2. Reactor 将 Event 交给 Demultiplex（epoll）进行监控
3. Demultiplex 检测到 Event 就绪，通知 Reactor
4. Reactor 将就绪的 Event 分发给对应的 EventHandler 处理

### 本项目的映射关系

| 抽象概念 | 对应类 | 说明 |
|----------|--------|------|
| Reactor | `EventLoop` | 每个线程一个，one thread one loop |
| Demultiplex | `Poller` | 封装 epoll，管理所有 fd 的事件监控 |
| Event | `Channel` | 封装 fd + 事件 + 回调 |
| EventHandler | 各类回调 | `read_callback_`、`write_callback_` 等 |

### 主从 Reactor 架构

单线程 Reactor 无法充分利用多核。本项目采用类似 Nginx 的架构：

- **Base Reactor（主）**：运行在主线程，通过 `Acceptor` 负责接收新连接
- **Sub Reactor（从）**：运行在线程池中，每个线程一个 `EventLoop`，负责已建立连接的 I/O 读写
- **轮转分配**：新连接通过 round-robin 方式分配给从属 Reactor

```
客户端连接 → Acceptor (base loop) → 轮转分配 → Sub Loop 1
                                                → Sub Loop 2
                                                → Sub Loop 3
```

---

## 模块详解

### CallbackTypes — 回调类型定义

各组件通过 `std::function` 定义回调类型，实现灵活的事件处理：

```cpp
using MessageCallback   = std::function<void(std::shared_ptr<Connection>, std::shared_ptr<Buffer>)>;
using NewConnectCallback = std::function<void(std::shared_ptr<Connection>)>;
using EventCallback     = std::function<void(std::shared_ptr<Connection>)>;
using CloseCallback     = std::function<void(std::shared_ptr<Connection>)>;

enum class ConnectionStatus {
    CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED
};
```

### Buffer — 用户态缓冲区

**功能**：对接收和发送的数据进行缓冲，通过读偏移和写偏移管理连续空间。

- `read_index_` 到 `write_index_` 是可读数据区域
- `buffer_.begin()` 到 `buffer_.begin() + write_index_` 是已占用空间
- 空间足够时移动数据并更新偏移；不够时扩容

**成员变量**：

| 变量 | 说明 |
|------|------|
| `std::vector<char> buffer_` | 连续内存空间 |
| `size_t read_index_` | 读偏移 |
| `size_t write_index_` | 写偏移 |

**关键接口**：

| 接口 | 说明 |
|------|------|
| `Read() / ReadAndPop()` | 读取数据（Pop 版本会移动读偏移） |
| `Write() / WriteAndPush()` | 写入数据（Push 版本会移动写偏移） |
| `GetLine() / GetLineAndPop()` | 获取以 `\n` 结尾的一行数据 |
| `Clear()` | 重置缓冲区状态 |
| `CheckCapcity()` | 内部扩容：先尝试移动数据腾出空间，不足再扩容 |

### Channel — 文件描述符事件管理

**功能**：对所有文件描述符（监听套接字、连接套接字、eventfd、timerfd）进行统一管理，将 epoll 的事件监控在用户态进行封装。

**成员变量**：

| 变量 | 说明 |
|------|------|
| `int fd_` | 管理的文件描述符 |
| `int events_` | 当前需要监控的事件（EPOLLIN/EPOLLOUT 等） |
| `int revents_` | 当前触发的事件 |
| `weak_ptr<EventLoop> eventloop_` | 关联的 EventLoop |
| 5 个 `Callback` | 读、写、错误、关闭、任意事件回调 |

**关键接口**：

| 接口 | 说明 |
|------|------|
| `ReadAble() / UnReadAble()` | 开启/关闭可读事件监控 |
| `WriteAble() / UnWriteAble()` | 开启/关闭可写事件监控 |
| `Handle()` | 根据 `revents_` 分发到对应回调 |
| `RemoveAllEvent()` | 移除所有事件监控 |

**Handle() 事件分发逻辑**：

```cpp
void Channel::Handle() {
    if (revents_ & EPOLLIN)       read_callback_();     // 可读
    if (revents_ & EPOLLOUT)      write_callback_();    // 可写
    if (revents_ & (EPOLLHUP|EPOLLERR))  error_callback_(); // 错误
    if (revents_ & EPOLLRDHUP)    close_callback_();    // 对端关闭
    if (event_callback_)          event_callback_();    // 任意事件
}
```

### Poller — epoll 封装（Demultiplex）

**功能**：封装 Linux epoll，作为多路事件分发器。内部维护一个 `fd → Channel` 的哈希表，管理所有被监控的文件描述符。

**成员变量**：

| 变量 | 说明 |
|------|------|
| `int epollfd_` | epoll 实例的文件描述符 |
| `unordered_map<int, shared_ptr<Channel>> channels_` | fd 到 Channel 的映射 |

**关键流程**：
1. `UpdateEvent(channel)` — 若 fd 不在 map 中则 `EPOLL_CTL_ADD`，否则 `EPOLL_CTL_MOD`
2. `RemvoeEvent(channel)` — `EPOLL_CTL_DEL` 并从 map 中移除
3. `EpollWait()` — 调用 `epoll_wait` 阻塞等待，返回就绪的 Channel 列表

### Socket — 套接字封装

**功能**：对 Linux socket API 的 RAII 封装。在 `Acceptor` 构造函数中通过 `CreateServer()` → `socket() → bind() → listen()` 完成服务端套接字初始化。

**关键接口**：

| 接口 | 说明 |
|------|------|
| `CreateServer(port, ip)` | 创建并绑定监听套接字 |
| `CreateClient(port, ip)` | 创建客户端连接 |
| `Accept()` | 接收新连接，返回 fd |
| `Recv() / Send()` | 阻塞收发 |
| `RecvNoBlock() / SendNoBlock()` | 非阻塞收发（`MSG_DONTWAIT`） |
| `ReuseAddr() / ReusePort()` | 开启地址/端口重用 |

### EventLoop — 事件循环（Reactor 核心）

**功能**：每个线程独占一个 EventLoop，是 Reactor 模式的核心组件。整合 Poller、Channel 和 TimerWheel，循环等待并处理 I/O 事件和任务队列。

**成员变量**：

| 变量 | 说明 |
|------|------|
| `std::thread::id id_` | 绑定线程 ID |
| `Poller poller_` | epoll 封装 |
| `int eventfd_` | 用于跨线程唤醒 |
| `shared_ptr<Channel> channel_` | eventfd 对应的 Channel |
| `vector<Task> tasks_` + `mutex_` | 跨线程任务队列 + 锁 |
| `unique_ptr<TimerWheel> timerwheel_` | 时间轮定时器 |

**eventfd 唤醒机制**：当其他线程需要向 EventLoop 提交任务时（如新连接到来、发送数据），通过 `PutIntoQueue()` 将任务放入队列并 `write(eventfd_)`，唤醒可能阻塞在 `epoll_wait` 中的 EventLoop。

**核心循环**（`HanleTask()`）：

```cpp
void EventLoop::HanleTask() {
    timerwheel_->Ready();      // 启动时间轮
    channel_->ReadAble();      // 监听 eventfd 唤醒
    while (1) {
        auto ready = poller_.EpollWait();   // 阻塞等待事件
        for (auto& channel : ready)
            channel->Handle();              // 分发事件
        // 处理跨线程任务
        lock_guard<mutex> lock(mutex_);
        for (auto& task : tasks_) task();
        tasks_.clear();
    }
}
```

**关键接口**：

| 接口 | 说明 |
|------|------|
| `RunInLoop(task)` | 若在当前线程则直接执行，否则放入队列 |
| `PutIntoQueue(task)` | 放入任务队列并唤醒 |
| `IsInLoop()` | 判断当前是否在 EventLoop 线程 |
| `AddTimedJob() / UpdateTimedJob() / CancelTimedJob()` | 定时任务管理 |

### TimerWheel — 时间轮定时器

**功能**：基于时间轮算法实现定时任务管理，用于连接超时销毁和定时任务调度。使用 Linux `timerfd` 驱动，精度为 1 秒。

**成员变量**：

| 变量 | 说明 |
|------|------|
| `vector<vector<shared_ptr<TimeTask>>> wheel_` | 60 槽的时间轮（每槽 1 秒，覆盖 60 秒） |
| `size_t tick_` | 当前指针位置 |
| `unordered_map<size_t, weak_ptr<TimeTask>> tasks_` | ID 到任务的映射 |
| `int timerfd_` | timerfd 描述符 |

**辅助类 TimeTask**：利用 RAII 实现定时任务——析构时自动执行回调（除非被 Cancel）。

**工作流程**：
1. `timerfd_settime` 设置 1 秒周期定时
2. 每次定时器触发，`tick_` 向前移动一格
3. 清空当前槽位的所有 `shared_ptr<TimeTask>`，触发析构回调
4. `UpdateTask()` 刷新：在任务所在 EventLoop 重新计算位置并加入轮子
5. `CancelTask()` 取消：标记 `is_cancel_` 防止析构时执行回调

### LoopThread & LoopThreadPool — 线程池

**LoopThread**：将 `std::thread` 和 `EventLoop` 绑定，实现 one thread one loop。通过互斥锁和条件变量同步，确保外部调用 `GetEventLoop()` 时 EventLoop 已初始化完成。

```cpp
void LoopThread::ThreadCallback() {
    eventloop_ = make_shared<EventLoop>();
    eventloop_->Init();
    cond_.notify_all();         // 通知等待者
    eventloop_->HanleTask();    // 进入事件循环（阻塞）
}
```

**LoopThreadPool**：管理多个 `LoopThread`，通过 round-robin 方式分配 EventLoop 给新连接。当线程数为 0 时，返回主 Reactor（base_loop）处理所有连接。

### Acceptor — 监听器

**功能**：属于主 Reactor，对监听套接字进行管理。当新连接到来时，监听套接字触发可读事件，Acceptor 调用 `accept()` 获取新连接 fd，然后通过回调通知 TcpServer 创建 Connection 并分发到从属 Reactor。

```cpp
void Acceptor::AcceptReadCallback() {
    int fd = socket_.Accept();
    if (fd < 0) return;
    if (new_connect_cb_) new_connect_cb_(fd);  // → TcpServer::AcceptCallback
}
```

### Connection — 连接管理

**功能**：对一个 TCP 连接的全生命周期管理。整合了 Socket、Channel、Buffer，并对外提供回调接口。

**成员变量**：

| 变量 | 说明 |
|------|------|
| `uint64_t conn_id_` | 连接唯一 ID |
| `shared_ptr<EventLoop> eventloop_` | 绑定的 EventLoop |
| `shared_ptr<Channel> channel_` | 连接 fd 对应的 Channel |
| `shared_ptr<Buffer> in_buffer_` | 输入缓冲区 |
| `shared_ptr<Buffer> out_buffer_` | 输出缓冲区 |
| `Socket socket_` | 连接套接字 |
| `ConnectionStatus status_` | 连接状态 |
| `std::any context_` | 协议上下文（HTTP 解析状态机） |
| 5 个回调 | message、newconnect、event、close、svr_close |
| `bool is_selfrelease_` | 是否启用超时自动释放 |

**关键流程**：

1. **读取数据**：`ConnReadCallback()` → `socket_.RecvNoBlock()` → `in_buffer_->WriteAndPush()` → 调用 `message_callback_`
2. **发送数据**：`SendMessage()` → `RunInLoop(SendMessageInLoop)` → `out_buffer_->WriteAndPush()` → 开启写事件监控
3. **写就绪**：`ConnWriteCallback()` → 从 `out_buffer_` 取出数据 → `socket_.SendNoBlock()` → 发完关闭写监控
4. **连接关闭**：`Release()` → 放入 EventLoop 队列 → `ReleaseInLoop()` → 移除监控、关闭 socket、调用回调
5. **超时机制**：`SetSelfRelease(timeout)` → 在 `Ready()` 中向 EventLoop 注册定时器，超时后自动调用 `Release()`

### TcpServer — 服务器整合

**功能**：整合所有模块的顶层类，对使用者暴露简洁的接口。

**成员变量**：

| 变量 | 说明 |
|------|------|
| `shared_ptr<EventLoop> eventloop_` | 主 Reactor |
| `Acceptor acceptor_` | 监听器 |
| `LoopThreadPoll threadpoll_` | 从属线程池 |
| `unordered_map<uint64_t, shared_ptr<Connection>> connections_` | 所有连接 |
| 4 个回调 | message、newconnect、event、close |
| `uint64_t next_id_` | 自增 ID（连接和定时任务共用） |

**启动流程**（`Run()`）：

```cpp
void TcpServer::Run() {
    threadpoll_.Run();    // 1. 创建从属线程池
    acceptor_.Listen();   // 2. 将监听套接字注册到主 Reactor
    eventloop_->HanleTask(); // 3. 启动主 Reactor 事件循环
}
```

**新连接处理**（`AcceptCallback()`）：

```cpp
void TcpServer::AcceptCallback(int fd) {
    auto next_loop = threadpoll_.NextEventLoop();  // 轮转分配
    auto conn = make_shared<Connection>(next_id_, next_loop, fd);
    conn->SetMessageCallback(message_callback_);
    conn->SetSvrCloseCallback(bind(&TcpServer::CloseConnection, this, _1));
    if (is_selfrelease_) conn->SetSelfRelease(timeout_);
    connections_.emplace(next_id_++, conn);
    conn->Ready();   // 启动读监控 + 注册超时定时器
}
```

### HttpRequest / HttpResponse — HTTP 数据结构

**HttpRequest**：存储 HTTP 请求的各个部分

| 字段 | 说明 |
|------|------|
| `method_` | 请求方法（GET/POST/PUT/DELETE） |
| `path_` | 请求路径 |
| `version_` | 协议版本（HTTP/1.0, HTTP/1.1） |
| `params_` | URL 参数（`?key=value`） |
| `headers_` | 请求头 |
| `body_` | 请求体 |
| `keepalive_` | 是否长连接 |

**HttpResponse**：构造 HTTP 响应

| 字段 | 说明 |
|------|------|
| `version_` | 协议版本 |
| `response_code_` | 状态码 |
| `headers_` | 响应头 |
| `body_` | 响应体 |
| `keepalive_` | 是否长连接 |
| `is_redirect_` + `redirect_url_` | 重定向支持 |

### HttpContext — HTTP 协议解析

**功能**：作为 Connection 的上下文（通过 `std::any` 存储），实现 HTTP 报文的增量解析。核心是状态机：

```
PARSELINE → PARSEHEADER → PARSEBODY → FINISH / ERROR
```

**解析流程**：

1. **ParseLine**：解析请求行 `GET /path?key=val HTTP/1.1`，使用正则提取 method、path、version 和 URL 参数
2. **ParseHeader**：逐行解析请求头（`Key: Value`），识别 `Connection: keep-alive` 和 `Content-Length`
3. **ParseBody**：根据 `Content-Length` 读取指定长度的请求体

*注：Parse() 中的 switch 故意不写 break，利用 fall-through 实现链式解析，一次调用尽可能推进多个状态。*

### HttpServer — HTTP 服务器

**功能**：在 TcpServer 基础上构建 HTTP 应用层，提供正则路由和静态文件服务。

**路由系统**：

```cpp
// 注册路由
server.Get("/hello", [](auto req, auto resp) {
    resp->SetBody("{\"msg\": \"hello\"}", "application/json");
});

// 路径参数
server.Get("/user", [](auto req, auto resp) {
    string name = req->GetParam("name");
    resp->SetBody("{\"user\": \"" + name + "\"}", "application/json");
});
```

**路由分发流程**：
1. 收到数据 → `HttpContext::Parse()` 解析 → 获取 `HttpRequest`
2. 检查是否为静态文件请求（`GET/HEAD` + 合法路径 + 文件存在）
3. 否则根据 HTTP Method（GET/POST/PUT/DELETE）查路由表
4. 用 `std::regex_match` 匹配路径，找到对应 handler 执行
5. 未匹配到路由 → 404 Not Found
6. `SendResponse()` 组装 HTTP 响应行、头、体 → `conn->SendMessage()`

**静态文件服务**：
- 设置 `base_dir_` 后，GET/HEAD 请求优先匹配静态文件
- 目录路径自动追加 `index.html`
- 包含路径穿越检测（`IsValidPath` 防止 `../` 攻击）
- MIME 类型通过文件扩展名自动识别（100+ 种类型）

### 工具类

| 类 | 说明 |
|----|------|
| `StringUtil` | 字符串分割（Spilt）、URL 编解码（UrlEncode/UrlDecode） |
| `FileUtil` | 文件读写、MIME 类型识别、路径合法性检测 |
| `HttpUtil` | HTTP 状态码 → 状态描述映射（100-511） |
| `LoggerManager` | 基于 quill 的异步日志封装，支持控制台/文件输出、日志级别、格式自定义 |

**日志宏**：

```cpp
QLOG_INFO("New connection: {} on port {}", conn_id, port);
QLOG_DEBUG("Recv {} bytes", n);
QLOG_WARN("Connection timeout: {}", conn_id);
QLOG_ERROR("Socket create fail: {}", strerror(errno));
```

---

## 总结

### 整体流程

1. 创建 `HttpServer`（内含 `TcpServer`）时自动创建 `EventLoop`（主 Reactor）、`Acceptor` 和 `LoopThreadPool`
2. 设置从属线程数量、注册路由、配置静态文件目录
3. 调用 `Run()` → `TcpServer::Run()`：
   - 创建从属线程池（每个线程一个 EventLoop）
   - `Acceptor::Listen()` 将监听套接字注册到主 Reactor 的 Poller
   - 启动主 Reactor 事件循环
4. 主 Reactor 在 `epoll_wait` 中等待新连接
5. 新连接到来 → `Acceptor::HandleRead()` → `accept()` → `TcpServer::AcceptCallback()`：
   - round-robin 分配一个从属 EventLoop
   - 创建 `Connection` 对象，设置回调
   - 存入 `connections_` map
   - `conn->Ready()` 启动读事件监控
6. 从属 Reactor 检测到连接可读 → `Connection::ConnReadCallback()` → 接收数据到 `in_buffer_` → 调用 `message_callback_`（即 `HttpServer::MessageCallback`）
7. `HttpServer::MessageCallback`：
   - 通过 `HttpContext` 增量解析 HTTP 报文
   - 解析完成后路由分发（静态文件 / 正则匹配）
   - 构造 `HttpResponse`
   - `SendResponse()` 组装 HTTP 报文 → `conn->SendMessage()` 发送
8. 连接关闭时：`Connection::Release()` → 移除 epoll 监控 → 关闭 socket → 调用关闭回调 → `TcpServer::CloseConnection()` 从 map 中移除

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
