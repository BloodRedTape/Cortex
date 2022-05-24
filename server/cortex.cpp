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
			RepoDir->ReadEntireFile(s_HistoryFilename).second
		)
	{}

	void Run() {
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

			if (req->Type == RequestType::Pull)
				SendDiffHistory(connection, req->AsPullRequest().TopHash);

			if (req->Type == RequestType::Push) {
				PushRequest push = req->AsPushRequest();
				if (push.Commits.size()) {
					if (push.Commits[0].Previous == History.HashLastCommit()){
						ApplyCommits(std::move(push.Commits));
						SendSuccess(connection);
					}else{
						SendDiffHistory(connection, push.Commits[0].Previous);
					}
				}
			}
		}
	}

	void ApplyCommits(std::vector<FileCommit> commits) {
		// Broadcast changes
		ApplyCommitsToDir(RepoDir.get(), commits);
		
		for (const auto &commit : commits)
			History.Add(commit.Action);
		RepoDir->WriteEntireFile(s_HistoryFilename, History.ToBinary());
	}

	void SendDiffHistory(TcpSocket &connection, Hash top_hash) {
		DiffResponce resp;
		for (auto commit : History)
			resp.Commits.push_back({commit});
					
		if(!Responce(resp).Send(connection))
			Error("Disconnected during sending");
	}

	void SendSuccess(TcpSocket &socket) {
		if(!Responce(SuccessResponce()).Send(socket))
			Error("Disconnected during sending");
	}
};


int main(){
	Server("W:\\Dev\\Cortex\\out\\server\\").Run();
	return 0;
}
