// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vmlib/Socket.hpp>
#include <cminor/machine/Error.hpp>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>    
#elif defined(__linux) || defined(__unix) || defined(__posix)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#else
#error unknown platform
#endif

namespace cminor { namespace vmlib {

using namespace cminor::machine;

#ifndef _WIN32
#define SOCKET int;
#endif

int gnu_tls_logging_level = 0; // 0 - 9

void SetGnuTlsLoggingLevel(int level)
{
    gnu_tls_logging_level = level;
}

class SocketTable
{
public:
    static void Init();
    static void Done();
    static SocketTable& Instance() { Assert(instance, "socket table not initialized"); return *instance; }
    ~SocketTable();
    int32_t CreateSocket();
    void BindSocket(int32_t socketHandle, int port);
    void ListenSocket(int32_t socketHandle, int backlog);
    int32_t AcceptSocket(int32_t socketHandle);
    void CloseSocket(int32_t socketHandle);
    void ShutdownSocket(int32_t socketHandle, ShutdownMode mode);
    int32_t ConnectSocket(const std::string& node, const std::string& service);
    int SendSocket(int32_t socketHandle, const char* buf, int len, int flags);
    int ReceiveSocket(int32_t socketHandle, char* buf, int len, int flags);
private:
    static std::unique_ptr<SocketTable> instance;
    const int32_t maxNoLockSocketHandles = 256;
    std::vector<SOCKET> sockets;
    std::unordered_map<int32_t, SOCKET> socketMap;
    std::atomic<int32_t> nextSocketHandle;
    std::mutex mtx;
    SocketTable();
    void InitSockets();
    void DoneSockets();
    bool socketsInitialized;
};

std::unique_ptr<SocketTable> SocketTable::instance;

void SocketTable::Init()
{
    instance.reset(new SocketTable());
}

void SocketTable::Done()
{
    instance.reset();
}

SocketTable::SocketTable() : socketsInitialized(false), nextSocketHandle(1)
{
    sockets.resize(maxNoLockSocketHandles);
}

SocketTable::~SocketTable()
{
    if (socketsInitialized)
    {
        DoneSockets();
    }
}

int GetLastSocketError()
{
#ifdef _WIN32
    return WSAGetLastError();
#elif defined(__linux) || defined(__unix) || defined(__posix)
    return errno;
#else
#error unknown platform
#endif
}

std::string GetSocketErrorMessage(int errorCode)
{
    char buf[1024];
#ifdef _WIN32
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, buf, sizeof(buf), NULL);
#elif defined(__linux) || defined(__unix) || defined(__posix)
    strncpy(buf, strerror(errorCode), sizeof(buf));
#else
#error unknown platform
#endif
    return std::string(buf);
}

void SocketTable::InitSockets()
{
    if (socketsInitialized) return;
#ifdef _WIN32
    WORD ver = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(ver, &wsaData) != 0)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
#endif
    //gnutls_global_set_log_level(gnu_tls_logging_level);
    socketsInitialized = true;
}

void SocketTable::DoneSockets()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

int32_t SocketTable::CreateSocket()
{
    InitSockets();
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _WIN32
    if (s == INVALID_SOCKET)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
#else
    if (s == -1)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
#endif
    int32_t socketHandle = nextSocketHandle++;
    if (socketHandle < maxNoLockSocketHandles)
    {
        sockets[socketHandle] = s;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        socketMap[socketHandle] = s;
    }
    return socketHandle;
}

void SocketTable::BindSocket(int32_t socketHandle, int port)
{
    InitSockets();
    int result = 0;
    if (socketHandle <= 0)
    {
        throw SocketError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port);
        result = bind(s, (struct sockaddr*) &addr, sizeof(addr));
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(port);
            result = bind(s, (struct sockaddr*) &addr, sizeof(addr));
        }
        else
        {
            throw SocketError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
}

void SocketTable::ListenSocket(int32_t socketHandle, int backlog)
{
    InitSockets();
    int result = 0;
    if (socketHandle <= 0)
    {
        throw SocketError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = listen(s, backlog);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = listen(s, backlog);
        }
        else
        {
            throw SocketError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
}

int32_t SocketTable::AcceptSocket(int32_t socketHandle)
{
    InitSockets();
    SOCKET a = 0;
    if (socketHandle <= 0)
    {
        throw SocketError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        a = accept(s, NULL, NULL);
#ifdef _WIN32
        if (a == INVALID_SOCKET)
        {
            int errorCode = GetLastSocketError();
            SocketError socketError(GetSocketErrorMessage(errorCode));
            socketError.SetErrorCode(errorCode);
            throw socketError;
        }
#else
        if (s == -1)
        {
            int errorCode = GetLastSocketError();
            SocketError socketError(GetSocketErrorMessage(errorCode));
            socketError.SetErrorCode(errorCode);
            throw socketError;
        }
#endif
        int32_t acceptedSocketHandle = nextSocketHandle++;
        if (acceptedSocketHandle < maxNoLockSocketHandles)
        {
            sockets[acceptedSocketHandle] = a;
        }
        else
        {
            std::lock_guard<std::mutex> lock(mtx);
            socketMap[acceptedSocketHandle] = a;
        }
        return acceptedSocketHandle;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            a = accept(s, NULL, NULL);
#ifdef _WIN32
            if (a == INVALID_SOCKET)
            {
                int errorCode = GetLastSocketError();
                SocketError socketError(GetSocketErrorMessage(errorCode));
                socketError.SetErrorCode(errorCode);
                throw socketError;
            }
#else
            if (s == -1)
            {
                int errorCode = GetLastSocketError();
                SocketError socketError(GetSocketErrorMessage(errorCode));
                socketError.SetErrorCode(errorCode);
                throw socketError;
            }
#endif
            int32_t acceptedSocketHandle = nextSocketHandle++;
            if (acceptedSocketHandle < maxNoLockSocketHandles)
            {
                sockets[acceptedSocketHandle] = a;
            }
            else
            {
                socketMap[acceptedSocketHandle] = a;
            }
            return acceptedSocketHandle;
        }
        else
        {
            throw SocketError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
}

void SocketTable::CloseSocket(int32_t socketHandle)
{
    InitSockets();
    int result = 0;
    if (socketHandle <= 0)
    {
        throw SocketError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
#ifdef _WIN32
        result = closesocket(s);
#else
        result = close(s);
#endif
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
#ifdef _WIN32
            result = closesocket(s);
#else
            result = close(s);
#endif
        }
        else
        {
            throw SocketError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
}

void SocketTable::ShutdownSocket(int32_t socketHandle, ShutdownMode mode)
{
    InitSockets();
    int result = 0;
#ifdef _WIN32    
    int how = SD_RECEIVE;
    switch (mode)
    {
        case ShutdownMode::receive: how = SD_RECEIVE; break;
        case ShutdownMode::send: how = SD_SEND; break;
        case ShutdownMode::both: how = SD_BOTH; break;
    }
#elif defined(__linux) || defined(__unix) || defined(__posix)
    int how = SHUT_RD;
    switch (mode)
    {
        case ShutdownMode::receive: how = SHUT_RD; break;
        case ShutdownMode::send: how = SHUT_WR; break;
        case ShutdownMode::both: how = SHUT_RDWR; break;
    }
#else
#error unknown platform
#endif    
    if (socketHandle <= 0)
    {
        throw SocketError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = shutdown(s, how);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = shutdown(s, how);
        }
        else
        {
            throw SocketError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
}

int32_t SocketTable::ConnectSocket(const std::string& node, const std::string& service)
{
    InitSockets();
    struct addrinfo hint;
    struct addrinfo* rp;
    struct addrinfo* res;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_flags = 0;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;
    hint.ai_addrlen = 0;
    hint.ai_addr = 0;
    hint.ai_canonname = 0;
    hint.ai_next = 0;
    int result = getaddrinfo(node.c_str(), service.c_str(), &hint, &res);
    if (result != 0)
    {
        SocketError socketError(gai_strerror(result));
        socketError.SetErrorCode(result);
        throw socketError;
    }
    else
    {
        for (rp = res; rp != 0; rp = rp->ai_next)
        {
            SOCKET s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (s == -1)
            {
                continue;
            }
            int result = connect(s, rp->ai_addr, (int)rp->ai_addrlen);
            if (result == 0)
            {
                freeaddrinfo(res);
                int32_t connectedSocketHandle = nextSocketHandle++;
                if (connectedSocketHandle < maxNoLockSocketHandles)
                {
                    sockets[connectedSocketHandle] = s;
                }
                else
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    socketMap[connectedSocketHandle] = s;
                }
                return connectedSocketHandle;
            }
            else
            {
                freeaddrinfo(res);
                int errorCode = GetLastSocketError();
                SocketError socketError(GetSocketErrorMessage(errorCode));
                socketError.SetErrorCode(errorCode);
                throw socketError;
            }
        }
    }
    throw SocketError("could not connect");
}

int SocketTable::SendSocket(int32_t socketHandle, const char* buf, int len, int flags)
{
    InitSockets();
    int result = 0;
    if (socketHandle <= 0)
    {
        throw SocketError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = send(s, buf, len, flags);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = send(s, buf, len, flags);
        }
        else
        {
            throw SocketError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result < 0)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
    return result;
}

int SocketTable::ReceiveSocket(int32_t socketHandle, char* buf, int len, int flags)
{
    InitSockets();
    int result = 0;
    if (socketHandle <= 0)
    {
        throw SocketError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = recv(s, buf, len, flags);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = recv(s, buf, len, flags);
        }
        else
        {
            throw SocketError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result < 0)
    {
        int errorCode = GetLastSocketError();
        SocketError socketError(GetSocketErrorMessage(errorCode));
        socketError.SetErrorCode(errorCode);
        throw socketError;
    }
    return result;
}

int32_t CreateSocket()
{
    return SocketTable::Instance().CreateSocket();
}

void BindSocket(int32_t socketHandle, int port)
{
    SocketTable::Instance().BindSocket(socketHandle, port);
}

void ListenSocket(int32_t socketHandle, int backlog)
{
    SocketTable::Instance().ListenSocket(socketHandle, backlog);
}

int32_t AcceptSocket(int32_t socketHandle)
{
    return SocketTable::Instance().AcceptSocket(socketHandle);
}

void CloseSocket(int32_t socketHandle)
{
    SocketTable::Instance().CloseSocket(socketHandle);
}

void ShutdownSocket(int32_t socketHandle, ShutdownMode mode)
{
    SocketTable::Instance().ShutdownSocket(socketHandle, mode);
}

int32_t ConnectSocket(const std::string& node, const std::string& service)
{
    return SocketTable::Instance().ConnectSocket(node, service);
}

int SendSocket(int32_t socketHandle, const char* buf, int len, int flags)
{
    return SocketTable::Instance().SendSocket(socketHandle, buf, len, flags);
}

int ReceiveSocket(int32_t socketHandle, char* buf, int len, int flags)
{
    return SocketTable::Instance().ReceiveSocket(socketHandle, buf, len, flags);
}

void SocketInit()
{
    SocketTable::Init();
}

void SocketDone()
{
    SocketTable::Done();
}

} } // namespace cminor::vmlib
