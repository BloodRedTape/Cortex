#pragma once

#include "net/socket.hpp"
#include "net/tcp_socket.hpp"

class TcpListener: public Socket{
private:
	SocketHandle m_Handle = InvalidSocket;
public:
	TcpListener();

	TcpListener(TcpListener &&)noexcept;

	~TcpListener();

	TcpListener &operator=(TcpListener &&)noexcept;

	bool Bind(IpAddress address, u16 port);

	TcpSocket Accept();

private:
	bool IsValid()const {
		return m_Handle != InvalidSocket;
	}

	void MakeInvalid();

	static bool ListenImpl(SocketHandle socket);

	static SocketHandle AcceptImpl(SocketHandle socket, IpAddress &src_ip, u16 &src_port_hbo);
};