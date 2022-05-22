#include "net/udp_socket.hpp"
#include "net/tcp_socket.hpp"
#include "net/tcp_listener.hpp"
#include "error.hpp"
#include <atomic>
#include <WinSock2.h>
#define WSA_EXPR(xpr) xpr; Info("Expr{%} => %", #xpr, WSAGetLastError()); (WSAGetLastError() != 0 ? __debugbreak() : (void)0)

static std::atomic<bool> s_IsWSAInited{false};

SocketHandle Socket::OpenImpl(bool is_udp) {
	if (!s_IsWSAInited.exchange(true)){
		WSADATA wsaData;
		WSA_EXPR((void)WSAStartup(MAKEWORD(2,2), &wsaData));
	}
	WSA_EXPR(SOCKET sock = socket(AF_INET, is_udp ? SOCK_DGRAM : SOCK_STREAM, 0));
	return (SocketHandle)sock;
}

void Socket::CloseImpl(SocketHandle socket) {
	WSA_EXPR(closesocket((SOCKET)socket));
}

bool Socket::BindImpl(SocketHandle socket, IpAddress address, u16 port_hbo) {
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = ToNetByteOrder(port_hbo);
	addr.sin_addr.s_addr = (u32)address;

	WSA_EXPR(bool res = (bind((SOCKET)socket, (sockaddr*)&addr, sizeof(addr)) == 0));

	return res;
}

void Socket::SetBlocking(SocketHandle socket, bool is_blocking) {
	u_long blocking = is_blocking ? 0 : 1;
    WSA_EXPR(ioctlsocket((SOCKET)socket, static_cast<long>(FIONBIO), &blocking));
}

u32 UdpSocket::SendImpl(SocketHandle socket, const void* data, u32 size, IpAddress dst_ip, u16 dst_port_hbo) {
	
	sockaddr_in dst_addr = {0};
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = ToNetByteOrder(dst_port_hbo);
	dst_addr.sin_addr.s_addr = (u32)dst_ip;

	WSA_EXPR(auto sent = sendto((SOCKET)socket, (const char*)data, size, 0, (sockaddr *)&dst_addr, sizeof(dst_addr)));

	if(sent == -1)
		return 0;
	return u32(sent);
}

u32 UdpSocket::ReceiveImpl(SocketHandle socket, void* data, u32 size, IpAddress& src_ip, u16& src_port_hbo) {
	sockaddr_in src_addr = {0};
	int addr_len = sizeof(src_addr);

	WSA_EXPR(auto received = recvfrom((SOCKET)socket, (char*)data, size, 0, (sockaddr*)&src_addr, &addr_len));
	
	if(received == -1)
		return 0;

	src_ip = IpAddress(src_addr.sin_addr.s_addr);
	src_port_hbo = ToHostByteOrder(src_addr.sin_port);

	return u32(received);
}

bool TcpSocket::ConnectImpl(SocketHandle socket, IpAddress address, u16 port_hbo) {
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_hbo);
	addr.sin_addr.s_addr = (u32)address;

	WSA_EXPR(bool res = connect((SOCKET)socket, (sockaddr*)&addr, sizeof(addr)) == 0);

	return res;
}

// if returned size is less than size(param) then the error occured during transfering
u32 TcpSocket::SendImpl(SocketHandle socket, const void* data, u32 size, bool& is_disconnected) {
	is_disconnected = false;

	u32 actual_sent = 0;

	do{
		WSA_EXPR(auto sent = send((SOCKET)socket, (const char*)data + actual_sent, size - actual_sent, 0));

		if(sent == -1){
			is_disconnected = true;
			break;
		}
		
		actual_sent += sent;

	}while(actual_sent < size);
	
	return actual_sent;
}

u32 TcpSocket::ReceiveImpl(SocketHandle socket, void* data, u32 size, bool& is_disconnected) {
	is_disconnected = false;

	u32 actual_received = 0;

	do {
		WSA_EXPR(auto received = recv((SOCKET)socket, (char*)data + actual_received, size - actual_received, 0));
		
		if (received == 0) {
			is_disconnected = true;
			break;
		}

		if (received == -1) {
			auto last_error = WSAGetLastError();
			is_disconnected = !(last_error == WSAEWOULDBLOCK || last_error == WSAEALREADY);
			break;
		}
		actual_received += received;

	}while(actual_received < size);

	return actual_received;
}

bool TcpListener::ListenImpl(SocketHandle socket) {
	return listen((SOCKET)socket, 20) == 0;
}

SocketHandle TcpListener::AcceptImpl(SocketHandle socket, IpAddress& src_ip, u16& src_port_hbo) {
	sockaddr_in src_addr = {0};
	int addr_len = sizeof(src_addr);
	WSA_EXPR(SOCKET connection = accept((SOCKET)socket, (sockaddr*)&src_addr, &addr_len));
	
	src_ip = IpAddress(src_addr.sin_addr.s_addr);
	src_port_hbo = ToHostByteOrder(src_addr.sin_port);		
	return (SocketHandle)connection;
}
