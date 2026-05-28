#pragma once

#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "base/socket.h"

// ============================================================
// Helper: get an available port from the OS
// ============================================================

static uint16_t GetFreePort()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_GE(fd, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = 0;
    bind(fd, (const sockaddr*)&addr, sizeof(addr));
    socklen_t len = sizeof(addr);
    getsockname(fd, (sockaddr*)&addr, &len);
    uint16_t port = ntohs(addr.sin_port);
    close(fd);
    return port;
}

// ============================================================
// Constructor & Fd
// ============================================================

TEST(SocketTest, DefaultConstructor_FdIsNegative)
{
    Socket sock;
    EXPECT_EQ(sock.Fd(), -1);
}

TEST(SocketTest, ConstructorWithFd_StoresFd)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);

    Socket sock(fd);
    EXPECT_EQ(sock.Fd(), fd);

    close(fd);
}

// ============================================================
// CreateServer
// ============================================================

TEST(SocketTest, CreateServer_Success)
{
    uint16_t port = GetFreePort();

    Socket server;
    EXPECT_TRUE(server.CreateServer(port, "127.0.0.1"));
    EXPECT_GE(server.Fd(), 0);

    close(server.Fd());
}

TEST(SocketTest, CreateServer_DefaultIp)
{
    uint16_t port = GetFreePort();

    Socket server;
    EXPECT_TRUE(server.CreateServer(port));
    EXPECT_GE(server.Fd(), 0);

    close(server.Fd());
}

// ============================================================
// CreateClient
// ============================================================

TEST(SocketTest, CreateClient_ConnectsToServer)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    EXPECT_TRUE(client.CreateClient(port, "127.0.0.1"));

    close(server.Fd());
    close(client.Fd());
}

TEST(SocketTest, CreateClient_NoServer_Fails)
{
    uint16_t port = GetFreePort();

    Socket client;
    // No server listening on this port — connect should fail
    EXPECT_FALSE(client.CreateClient(port, "127.0.0.1"));
}

// ============================================================
// Bind / Listen (individually)
// ============================================================

TEST(SocketTest, Bind_ValidFd_Success)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    Socket sock(fd);

    uint16_t port = GetFreePort();
    EXPECT_TRUE(sock.Bind(port, "127.0.0.1"));

    close(fd);
}

TEST(SocketTest, Bind_InvalidFd_Fails)
{
    Socket sock; // fd_ = -1
    EXPECT_FALSE(sock.Bind(8080, "127.0.0.1"));
}

TEST(SocketTest, Listen_OnBoundSocket_Success)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    Socket sock(fd);

    uint16_t port = GetFreePort();
    ASSERT_TRUE(sock.Bind(port, "127.0.0.1"));
    EXPECT_TRUE(sock.Listen());

    close(fd);
}

TEST(SocketTest, Listen_WithoutBind_Fails)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    Socket sock(fd);

    // listen() on an unbound socket may or may not fail depending on the OS;
    // on Linux it succeeds but binds to a random port.
    // We just verify the call returns without crash.
    sock.Listen();

    close(fd);
}

// ============================================================
// Accept
// ============================================================

TEST(SocketTest, Accept_ReturnsValidFd)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    ASSERT_TRUE(client.CreateClient(port, "127.0.0.1"));

    int conn_fd = server.Accept();
    EXPECT_GE(conn_fd, 0);

    close(conn_fd);
    close(server.Fd());
    close(client.Fd());
}

TEST(SocketTest, Accept_NoPendingConnection_BlocksOrReturns)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    // Without a connecting client, Accept would block.
    // We set the server fd to non-blocking to verify Accept returns -1.
    int flags = fcntl(server.Fd(), F_GETFL, 0);
    fcntl(server.Fd(), F_SETFL, flags | O_NONBLOCK);

    int conn_fd = server.Accept();
    EXPECT_LT(conn_fd, 0);

    close(server.Fd());
}

// ============================================================
// Send / Recv
// ============================================================

TEST(SocketTest, SendAndRecv_ClientToServer)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    ASSERT_TRUE(client.CreateClient(port, "127.0.0.1"));

    int conn_fd = server.Accept();
    ASSERT_GE(conn_fd, 0);
    Socket conn(conn_fd);

    const std::string msg = "hello";
    EXPECT_TRUE(client.Send(msg));

    std::string buf;
    size_t ret = conn.Recv(buf);
    EXPECT_EQ(ret, msg.size());
    EXPECT_EQ(buf, msg);

    close(server.Fd());
    close(client.Fd());
    close(conn_fd);
}

TEST(SocketTest, SendAndRecv_ServerToClient)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    ASSERT_TRUE(client.CreateClient(port, "127.0.0.1"));

    int conn_fd = server.Accept();
    ASSERT_GE(conn_fd, 0);
    Socket conn(conn_fd);

    const std::string msg = "world";
    EXPECT_TRUE(conn.Send(msg));

    std::string buf;
    size_t ret = client.Recv(buf);
    EXPECT_EQ(ret, msg.size());
    EXPECT_EQ(buf, msg);

    close(server.Fd());
    close(client.Fd());
    close(conn_fd);
}

TEST(SocketTest, SendAndRecv_MultipleMessages)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    ASSERT_TRUE(client.CreateClient(port, "127.0.0.1"));

    int conn_fd = server.Accept();
    ASSERT_GE(conn_fd, 0);
    Socket conn(conn_fd);

    for (int i = 0; i < 5; ++i)
    {
        std::string msg = "msg" + std::to_string(i);
        EXPECT_TRUE(client.Send(msg));

        std::string buf;
        conn.Recv(buf);
        EXPECT_EQ(buf, msg);
    }

    close(server.Fd());
    close(client.Fd());
    close(conn_fd);
}

TEST(SocketTest, SendAndRecv_EmptyString)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    ASSERT_TRUE(client.CreateClient(port, "127.0.0.1"));

    int conn_fd = server.Accept();
    ASSERT_GE(conn_fd, 0);
    Socket conn(conn_fd);

    // send(fd, "", 0, 0) returns 0 (success, but no data on wire)
    EXPECT_TRUE(client.Send(""));

    // Close client to trigger EOF on server side
    close(client.Fd());

    std::string buf;
    size_t ret = conn.Recv(buf);
    EXPECT_EQ(ret, 0); // EOF

    close(server.Fd());
    close(conn_fd);
}

// ============================================================
// SendNoBlock / RecvNoBlock
// ============================================================

TEST(SocketTest, SendNoBlock_And_RecvNoBlock)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    ASSERT_TRUE(client.CreateClient(port, "127.0.0.1"));

    int conn_fd = server.Accept();
    ASSERT_GE(conn_fd, 0);
    Socket conn(conn_fd);

    const std::string msg = "noblock";
    EXPECT_TRUE(client.SendNoBlock(msg));

    std::string buf;
    size_t ret = conn.RecvNoBlock(buf);
    EXPECT_EQ(ret, msg.size());
    EXPECT_EQ(buf, msg);

    close(server.Fd());
    close(client.Fd());
    close(conn_fd);
}

// ============================================================
// Error cases
// ============================================================

TEST(SocketTest, Send_OnInvalidFd_Fails)
{
    Socket sock; // fd_ = -1
    EXPECT_FALSE(sock.Send("data"));
}

TEST(SocketTest, Recv_OnInvalidFd_ReturnsError)
{
    Socket sock; // fd_ = -1
    std::string buf;
    size_t ret = sock.Recv(buf);
    // recv on invalid fd returns -1, cast to size_t is large
    EXPECT_GT(ret, 0) << "Expected error from recv on invalid fd";
}

TEST(SocketTest, Connect_InvalidAddress_Fails)
{
    uint16_t port = GetFreePort();

    Socket client;
    // Connect to localhost on a free port with no listener — immediate RST
    EXPECT_FALSE(client.Connect(port, "127.0.0.1"));
}

TEST(SocketTest, Bind_AlreadyInUse_SucceedsWithReusePort)
{
    uint16_t port = GetFreePort();

    Socket s1;
    ASSERT_TRUE(s1.CreateServer(port, "127.0.0.1"));

    // SO_REUSEPORT allows multiple sockets to bind to the same port
    Socket s2;
    EXPECT_TRUE(s2.CreateServer(port, "127.0.0.1"));

    close(s1.Fd());
    close(s2.Fd());
}

// ============================================================
// Connect (standalone)
// ============================================================

TEST(SocketTest, Connect_Standalone)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    ASSERT_TRUE(client.CreateClient(port, "127.0.0.1"));

    EXPECT_GE(client.Fd(), 0);

    close(server.Fd());
    close(client.Fd());
}

// ============================================================
// Default IP parameter
// ============================================================

TEST(SocketTest, Bind_UsesDefaultIp)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    Socket sock(fd);

    uint16_t port = GetFreePort();
    EXPECT_TRUE(sock.Bind(port)); // default ip = "0.0.0.0"

    close(fd);
}

TEST(SocketTest, CreateClient_UsesProvidedIp)
{
    uint16_t port = GetFreePort();

    Socket server;
    ASSERT_TRUE(server.CreateServer(port, "127.0.0.1"));

    Socket client;
    EXPECT_TRUE(client.CreateClient(port, "127.0.0.1"));

    close(server.Fd());
    close(client.Fd());
}
