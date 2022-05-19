#include <iostream>
#include "fs/dir.hpp"
#include "net/tcp_socket.hpp"
#include "commit.hpp"

struct Host {
	IpAddress Address;
};

class Server {
private:
	// The Truth is here
	CommitHistory History;
public:
	
	void OnRequest(const Request& request, Responce& responce) {

	}
};

class Client {
	CommitHistory History;
	std::vector<FileAction> LocalChanges;
public:

	void PushChanges() { }

	void PullHistory() { }
};


int main() {
}