#include "net/ip.hpp"
#include <cstdio>

const IpAddress IpAddress::ThisNetworkBroadcast(255, 255, 255, 255);
const IpAddress IpAddress::Loopback(127, 0, 0, 1);
const IpAddress IpAddress::Any(0, 0, 0, 0);

IpAddress::IpAddress(const char* address){
	int octets[4];
	int res = sscanf(address, "%d.%d.%d.%d", octets + 0, octets + 1, octets + 2, octets + 3);
	CX_ASSERT(res == 4);

	*this = IpAddress(octets[0], octets[1], octets[2], octets[3]);
}

std::ostream &operator<<(std::ostream& stream, IpAddress address){
	u32 host = ToHostByteOrder((u32)address);

	stream << ((host & 0xFF000000) >> 24) 
	<< '.' << ((host & 0x00FF0000) >> 16)
	<< '.' << ((host & 0x0000FF00) >> 8 )
	<< '.' << ((host & 0x000000FF) >> 0 );

	return stream;
}
