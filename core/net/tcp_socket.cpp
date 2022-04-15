#include "net/tcp_socket.hpp"
#include <utility>

TcpSocket::TcpSocket(TcpSocket&& other){
	*this = std::move(other);
}

TcpSocket::~TcpSocket(){
	Disconnect();
}

TcpSocket& TcpSocket::operator=(TcpSocket&& other){
	Disconnect();
	std::swap(m_Handle, other.m_Handle);
	std::swap(m_RemoteIpAddress, other.m_RemoteIpAddress);
	std::swap(m_RemotePort, other.m_RemotePort);
	return *this;
}

bool TcpSocket::Connect(IpAddress address, u16 port_hbo){
	return ConnectImpl(m_Handle, address, port_hbo);
}

void TcpSocket::Disconnect(){
	MakeInvalid();
}

u32 TcpSocket::Send(const void* data, u32 size){
	bool is_disconnected = false;

	u32 sent = SendImpl(m_Handle, data, size, is_disconnected);
	
	if(is_disconnected)
		Disconnect();

	return sent;
}

u32 TcpSocket::Receive(void* data, u32 size){
	bool is_disconnected = false;
	
	u32 received = ReceiveImpl(m_Handle, data, size, is_disconnected);

	if(is_disconnected)
		Disconnect();

	return received;
}

bool TcpSocket::IsConnected()const{
	return RemotePort();
}

u16 TcpSocket::RemotePort() const{
	return m_RemotePort;
}

IpAddress TcpSocket::RemoteIpAddress() const{
	return m_RemoteIpAddress;
}

void TcpSocket::MakeValid(){
	if(IsValid())
		return;

	m_Handle = Socket::OpenImpl();
}

void TcpSocket::MakeInvalid(){
	if(!IsValid())
		return;

	Socket::CloseImpl(m_Handle);
	m_Handle = Socket::InvalidSocket;
	m_RemoteIpAddress = IpAddress::Any;
	m_RemotePort = 0;
}

bool TcpSocket::IsValid() const{
	return m_Handle != Socket::InvalidSocket;
}

