#include "net/udp_socket.hpp"
#include <atomic>
#include <WinSock2.h>

static std::atomic<bool> s_IsWSAInited{false};

SocketHandle Socket::OpenImpl() {
	if (!s_IsWSAInited.exchange(true)){
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,2), &wsaData);
	}

	return (SocketHandle)socket(AF_INET, SOCK_DGRAM, 0);
}

void Socket::CloseImpl(SocketHandle socket) {
	closesocket((SOCKET)socket);
}

bool Socket::BindImpl(SocketHandle socket, IpAddress address, u16 port_hbo) {
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = ToNetByteOrder(port_hbo);
	addr.sin_addr.s_addr = (u32)address;

	return bind((SOCKET)socket, (sockaddr*)&addr, sizeof(addr)) == 0;
}

u32 UdpSocket::SendImpl(SocketHandle socket, const void* data, u32 size, IpAddress dst_ip, u16 dst_port_hbo) {
	
	sockaddr_in dst_addr = {0};
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = ToNetByteOrder(dst_port_hbo);
	dst_addr.sin_addr.s_addr = (u32)dst_ip;

	auto sent = sendto((SOCKET)socket, (const char*)data, size, 0, (sockaddr *)&dst_addr, sizeof(dst_addr));

	if(sent < 0)
		return 0;
	return u32(sent);
}

u32 UdpSocket::ReceiveImpl(SocketHandle socket, void* data, u32 size, IpAddress& src_ip, u16& src_port_hbo) {
	sockaddr_in src_addr = {0};
	int addr_len = sizeof(src_addr);

	auto received = recvfrom((SOCKET)socket, (char*)data, size, 0, (sockaddr*)&src_addr, &addr_len);
	
	if(received < 0)
		return 0;

	src_ip = IpAddress(src_addr.sin_addr.s_addr);
	src_port_hbo = ToHostByteOrder(src_addr.sin_port);

	return u32(received);
}

