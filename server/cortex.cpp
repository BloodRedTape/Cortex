#include <fstream>
#include "protocol.hpp"
#include "fs/dir.hpp"
#include "commit.hpp"
#include "net/tcp_listener.hpp"
#include "net/udp_socket.hpp"
#include "error.hpp"

constexpr const char *s_HistoryFilename = ".history";

class Server {
private:
	DirRef RepoDir;
	CommitHistory History;

	TcpListener Listener;
	UdpSocket Broadcaster;
public:
	Server(std::string path):
		RepoDir(
			Dir::Create(std::move(path))
		),
		History(
			RepoDir->ReadEntireFile(s_HistoryFilename).second
		)
	{}

	void Run() {
		Broadcaster.Bind(IpAddress::LocalNetworkAddress(), Socket::AnyPort);
		Listener.Bind(IpAddress::Any, 25565);
		while (true) {
			TcpSocket connection     = Listener.Accept();
			IpAddress remote_address = connection.RemoteIpAddress();
			u16       remote_port    = connection.RemotePort();

			std::optional<Request> req = Request::Receive(connection);

			if (!req){
				Error("Disconnected during receiving");
				continue;
			}

			Println("Transition from: %:% -> %", remote_address, remote_port, RequestTypeString(req->Type));

			if (req->Type == RequestType::Pull)
				SendDiffHistory(connection, req->AsPullRequest().TopHash);

			if (req->Type == RequestType::Push) {
				PushRequest push = req->AsPushRequest();
				if (push.TopHash == History.HashLastCommit()) {
					ApplyActions(RepoDir.get(), History, push.Actions, push.ResultingFiles);
					//If remote address is loopback, then client is on this machine and loopback may be set as remote address
					BroadcastChanges(remote_address == IpAddress::Loopback ? IpAddress::LocalNetworkAddress() : remote_address);
					SendSuccess(connection, History.CollectCommitsAfter(push.TopHash));
				} else {
					SendDiffHistory(connection, push.TopHash);
				}
			}

			RepoDir->WriteEntireFile(s_HistoryFilename, History.ToBinary());
		}
	}

	void SendDiffHistory(TcpSocket &connection, Hash top_hash) {
		std::vector<Commit> commits = History.CollectCommitsAfter(top_hash);
		FileActionAccumulator accum;
		for(const auto &commit: commits)
			accum.Add(commit.Action);

		DiffResponce resp;
		resp.ResultingFiles = CollectFilesData(RepoDir.get(), accum);
		resp.Commits = std::move(commits);
					
		if(!Responce(resp).Send(connection))
			Error("Disconnected during sending");
	}

	void SendSuccess(TcpSocket &socket, std::vector<Commit> commits) {
		if(!Responce(SuccessResponce{commits}).Send(socket))
			Error("Disconnected during sending");
	}

	void BroadcastChanges(IpAddress src_address) {
		BroadcastProtocolHeader header{
			BroadcastMagicWord,
			src_address
		};

		//XXX: Validation
		Broadcaster.Send(&header, sizeof(header), IpAddress::ThisNetworkBroadcast, BroadcastListenPort);
	}
};


int main(){
	std::fstream config("server.config");
	CX_ASSERT(config.is_open());
	std::string filepath;
	std::getline(config, filepath);
	Server(std::move(filepath)).Run();
	return 0;
}
