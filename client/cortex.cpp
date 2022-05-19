#include <iostream>
#include "fs/dir.hpp"
#include "net/udp_socket.hpp"
#include "commit.hpp"

struct Host {
	IpAddress Address;
};

class Server {
private:
	// The Truth is here
	CommitHistory History;
public:
	void OnPullRequest(Host host){ 
		SendHistory(host);
	}

	void OnPushRequest(Host src, Hash top_commit, std::vector<FileAction> actions){ 
		if (History.HashLastCommit() == top_commit) {
			//Apply
			SendOk(src);
			BroadcastHistoryChanged();
		} else {
			SendHistory(src);
		}
	}

	void BroadcastHistoryChanged() {
		
	}

	void SendHistory(Host host){ }

	void SendOk(Host host){ }
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