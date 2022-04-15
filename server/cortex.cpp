#include "client_repository.hpp"
#include "net/tcp_socket.hpp"

int main(){
	//ClientRepository repo("W:\\Dev\\Cortex\\out\\repo\\");
	//repo.Run();

	TcpSocket Socket;
	Socket.Connect(IpAddress::Loopback, 25565);


	return 0;
}
