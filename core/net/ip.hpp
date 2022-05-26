#ifndef CORTEX_IP_ADDRESS_HPP
#define CORTEX_IP_ADDRESS_HPP

#include "utils.hpp"
#include "net/byte_order.hpp"
#include <ostream>

constexpr u32 IpHeaderSize = 20;
constexpr u32 MaxIpPacketSize = 0xFFFF;

class IpAddress {
public:
	static const IpAddress ThisNetworkBroadcast;
	static const IpAddress Loopback;
	static const IpAddress Any;
private:
	u32 m_Address = 0;
public:
	IpAddress(u32 network_byte_order_address):
		m_Address(network_byte_order_address)
	{}

	IpAddress(u8 o0, u8 o1, u8 o2, u8 o3):
		IpAddress(
			ToNetByteOrder(u32(o3) | u32(o2 << 8) | u32(o1 << 16) | u32(o0 << 24))
		)
	{}

	operator u32()const{
		return m_Address;
	}

	static IpAddress LocalNetworkAddress();

	friend std::ostream &operator<<(std::ostream &stream, IpAddress address);
};

#endif//CORTEX_IP_ADDRESS_HPP