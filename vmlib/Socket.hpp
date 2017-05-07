// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_VMLIB_SOCKET_INCLUDED
#define CMINOR_VMLIB_SOCKET_INCLUDED
#include <stdint.h>
#include <string>

namespace cminor { namespace vmlib {

enum class ShutdownMode
{
    receive = 0, send = 1, both = 2
};

int32_t CreateSocket();
void BindSocket(int32_t socketHandle, int port);
void ListenSocket(int32_t socketHandle, int backlog);
int32_t AcceptSocket(int32_t socketHandle);
void CloseSocket(int32_t socketHandle);
void ShutdownSocket(int32_t socketHandle, ShutdownMode mode);
int32_t ConnectSocket(const std::string& node, const std::string& service);
int SendSocket(int32_t socketHandle, const char* buf, int len, int flags);
int ReceiveSocket(int32_t socketHandle, char* buf, int len, int flags);

void SocketInit();
void SocketDone();

} } // namespace cminor::vmlib

#endif // CMINOR_VMLIB_SOCKET_INCLUDED
