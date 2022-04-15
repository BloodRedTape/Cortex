#ifndef CORTEX_TCP_SOCKET_HPP
#define CORTEX_TCP_SOCKET_HPP

#include "net/socket.hpp"

class TcpSocket: public Socket{
private:
	SocketHandle m_Handle = InvalidSocket;
	IpAddress m_RemoteIpAddress = IpAddress::Any;
	u16 m_RemotePort = 0;
public:
	TcpSocket() = default;

	TcpSocket(TcpSocket &&);

	~TcpSocket();

	TcpSocket &operator=(TcpSocket &&);

	bool Connect(IpAddress address, u16 port_hbo);

	void Disconnect();

	u32 Send(const void *data, u32 size);

	u32 Receive(void *data, u32 size);

	bool IsConnected()const;

	u16 RemotePort()const;

	IpAddress RemoteIpAddress()const;
private:
	void MakeValid();

	void MakeInvalid();

	bool IsValid()const;

	static bool ConnectImpl(SocketHandle socket, IpAddress address, u16 port_hbo);

	static u32 SendImpl(SocketHandle socket, const void *data, u32 size, bool &is_disconnected);

	static u32 ReceiveImpl(SocketHandle socket, void *data, u32 size, bool &is_disconected);
};

#endif//CORTEX_TCP_SOCKET_HPP