#include "protocol.hpp"
#include "fs/dir.hpp"
#include "commit.hpp"
#include "net/tcp_listener.hpp"
#include "error.hpp"

constexpr const char *s_HistoryFilename = ".history";

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
				if (push.TopHash == History.HashLastCommit()) {
					ApplyActionsToDir(RepoDir.get(), push.Actions, push.ResultingFiles);
					History.Add(push.Actions);
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
};


int main(){
	Server("W:\\Dev\\Cortex\\out\\server\\").Run();
	return 0;
}
