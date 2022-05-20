#include "repository.hpp"
#include "protocol.hpp"
#include "fs/dir.hpp"
#include "commit.hpp"
#include "net/tcp_socket.hpp"

class Server {
private:
	DirRef RepoDir;
	CommitHistory History;
public:
	Server(std::string path):
		RepoDir(
			Dir::Create(std::move(path))
		),
		History(
			RepoDir->ReadEntireFile(".history").second
		)
	{}

	~Server() {
		RepoDir->WriteEntireFile(".history", History.ToBinary());
	}

	void Run() {

	}
};


int main(){
	Server("W:\\Dev\\Cortex\\out\\server\\").Run();
	return 0;
}
