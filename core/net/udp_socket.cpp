#include "net/udp_socket.hpp"
#include <utility>
#include <cassert>

UdpSocket::UdpSocket() {
	Open();
}

UdpSocket::UdpSocket(UdpSocket&& other)noexcept{
	*this = std::move(other);
}

UdpSocket::~UdpSocket(){
	Close();
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other)noexcept{
	Close();
	std::swap(m_Handle, other.m_Handle);
	std::swap(m_IsBound, other.m_IsBound);
	return *this;
}

void UdpSocket::Open() {
	if(IsOpen())
		return;

	m_Handle = OpenImpl(true);
}

void UdpSocket::Close(){
	if(!IsOpen())
		return;

	CloseImpl(m_Handle);
	m_Handle = InvalidSocket;
}

bool UdpSocket::Bind(IpAddress address, u16 port_hbo) {
	assert(IsOpen());
	assert(!IsBound());

	return (m_IsBound = BindImpl(m_Handle, address, port_hbo));
}

bool UdpSocket::IsOpen()const{
	return m_Handle != InvalidSocket;
}

bool UdpSocket::IsBound()const {
	return m_IsBound;
}

u32 UdpSocket::Send(const void* data, u32 size, IpAddress dst_ip, u16 dst_port_hbo) {
	assert(IsOpen());
	m_IsBound = true;

	return SendImpl(m_Handle, data, size, dst_ip, dst_port_hbo);
}

u32 UdpSocket::Receive(void* data, u32 size, IpAddress& src_ip, u16& src_port_hbo) {
	assert(IsOpen());
	assert(IsBound());

	return ReceiveImpl(m_Handle, data, size, src_ip, src_port_hbo);
}
