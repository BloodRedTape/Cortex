#include "net/udp_socket.hpp"
#include "net/tcp_socket.hpp"
#include "net/tcp_listener.hpp"
#include "error.hpp"
#include <atomic>
#include <unistd.h>   
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

IpAddress IpAddress::LocalNetworkAddress() {
    char ac[80];
    if (gethostname(ac, sizeof(ac)))
        return IpAddress::Any;

    struct hostent *phe = gethostbyname(ac);

    if (phe == 0)
        return IpAddress::Any;

    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr = {0};
        std::memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		return IpAddress(addr.s_addr);
    }
	return IpAddress::Any;
}

SocketHandle Socket::OpenImpl(bool is_udp) {
	int sock = socket(AF_INET, is_udp ? SOCK_DGRAM : SOCK_STREAM, 0);

	if(is_udp){
		char broadcastEnable = 1;
		//XXX: Validation
		int ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	}

	return (SocketHandle)sock;
}

void Socket::CloseImpl(SocketHandle socket) {
	close((int)socket);
}

bool Socket::BindImpl(SocketHandle socket, IpAddress address, u16 port_hbo) {
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = ToNetByteOrder(port_hbo);
	addr.sin_addr.s_addr = (u32)address;

	return bind((int)socket, (sockaddr*)&addr, sizeof(addr)) == 0;
}

void Socket::SetBlocking(SocketHandle socket, bool is_blocking) {
    const int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, is_blocking ? flags ^ O_NONBLOCK : flags | O_NONBLOCK);
}

u32 UdpSocket::SendImpl(SocketHandle socket, const void* data, u32 size, IpAddress dst_ip, u16 dst_port_hbo) {
	
	sockaddr_in dst_addr = {0};
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = ToNetByteOrder(dst_port_hbo);
	dst_addr.sin_addr.s_addr = (u32)dst_ip;

	auto sent = sendto((int)socket, (const char*)data, size, 0, (sockaddr *)&dst_addr, sizeof(dst_addr));

	if(sent == -1)
		return 0;
	return u32(sent);
}

u32 UdpSocket::ReceiveImpl(SocketHandle socket, void* data, u32 size, IpAddress& src_ip, u16& src_port_hbo) {
	sockaddr_in src_addr = {0};
	socklen_t addr_len = sizeof(src_addr);

	auto received = recvfrom((int)socket, (char*)data, size, 0, (sockaddr*)&src_addr, &addr_len);
	
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

	return connect((int)socket, (sockaddr*)&addr, sizeof(addr));
}

// if returned size is less than size(param) then the error occured during transfering
u32 TcpSocket::SendImpl(SocketHandle socket, const void* data, u32 size, bool& is_disconnected) {
	is_disconnected = false;

	u32 actual_sent = 0;

	do{
		auto sent = send((int)socket, (const char*)data + actual_sent, size - actual_sent, 0);

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
		auto received = recv((int)socket, (char*)data + actual_received, size - actual_received, 0);
		
		if (received == 0) {
			is_disconnected = true;
			break;
		}

		if (received == -1) {
			is_disconnected = !(errno == EWOULDBLOCK || errno == EALREADY);
			break;
		}
		actual_received += received;

	}while(actual_received < size);

	return actual_received;
}

bool TcpListener::ListenImpl(SocketHandle socket) {
	return listen((int)socket, 20) == 0;
}

SocketHandle TcpListener::AcceptImpl(SocketHandle socket, IpAddress& src_ip, u16& src_port_hbo) {
	sockaddr_in src_addr = {0};
	socklen_t addr_len = sizeof(src_addr);
	int connection = accept((int)socket, (sockaddr*)&src_addr, &addr_len);
	
	src_ip = IpAddress(src_addr.sin_addr.s_addr);
	src_port_hbo = ToHostByteOrder(src_addr.sin_port);		
	return (SocketHandle)connection;
}
