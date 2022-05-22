#include <iostream>
#include <sstream>
#include "fs/dir.hpp"
#include "net/tcp_socket.hpp"
#include "protocol.hpp"
#include "error.hpp"

class Client {
private:
	TcpSocket Socket;
public:
	

	void Run(IpAddress server_address, u16 server_port) {
		while(true){
			if (!Socket.Connect(server_address, server_port))
				continue;
			Info("[%:%]: Connected", server_address, server_port);

			TransitionProtocolHeader header;
			header.TransitionSize = 24;

			if (!Socket.Send(&header, sizeof(header))){
				Info("Can't send");
				continue;
			}
		}
	}
};

int main() {
	Client().Run(IpAddress::Loopback, 25565);
}