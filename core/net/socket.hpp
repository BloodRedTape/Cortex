#ifndef CORTEX_SOCKET_HPP
#define CORTEX_SOCKET_HPP

#include "utils.hpp"
#include "net/ip.hpp"

using SocketHandle = u64;

class Socket {
public:
	static constexpr u16 AnyPort = 0;
protected:
	static constexpr SocketHandle InvalidSocket = -1;
protected:
	static SocketHandle OpenImpl();

	static void CloseImpl(SocketHandle socket);

	static bool BindImpl(SocketHandle socket, IpAddress address, u16 port_hbo);
};

#endif//CORTEX_SOCKET_HPP