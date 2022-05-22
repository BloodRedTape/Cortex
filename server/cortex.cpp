#include "repository.hpp"
#include "protocol.hpp"
#include "fs/dir.hpp"
#include "commit.hpp"
#include "net/tcp_listener.hpp"
#include "error.hpp"

class Server {
private:
	DirRef RepoDir;
	CommitHistory History;

	TcpListener Listener;
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
		Listener.Bind(IpAddress::Any, 25565);
		while (true) {
			TcpSocket connection     = Listener.Accept();
			IpAddress remote_address = connection.RemoteIpAddress();
			u16       remote_port    = connection.RemotePort();

			TransitionProtocolHeader header;

			u32 received = connection.Receive(&header, sizeof(header));
			
			Info("[%:%]: Got % bytes", remote_address, remote_port, received);
			if(!received || !connection.IsConnected()){
				Warning("[%:%]: Disconnected during receiving", remote_address, remote_port);
				continue;
			}

			if(header.MagicWord != TransitionMagicWord){
				Warning("[%:%]: Corrupted data", remote_address, remote_port);
				continue;
			}

			HandleTransition(std::move(connection));
		}
	}

	void HandleTransition(TcpSocket transition) {
		IpAddress remote_address = transition.RemoteIpAddress();
		u16       remote_port    = transition.RemotePort();
		Info("[%:%]: TransitionStarted", remote_address, remote_port);

		Info("[%:%]: TransitionFinished", remote_address, remote_port);
	}
};


int main(){
	Server("W:\\Dev\\Cortex\\out\\server\\").Run();
	return 0;
}
