#pragma once

#include "utils.hpp"
#include "net/ip.hpp"
#include "commit.hpp"
#include "fs/dir_watcher.hpp"

constexpr const char *s_StateFilename= ".state";

struct ClientState {
	CommitHistory History;
	FileActionAccumulator LocalChanges;
	DirState CurrentDirState;

	ClientState(std::string binary);

	std::string ToBinary()const;
};

class Client {
private:
	DirRef m_RepositoryDir;
	DirWatcherRef m_DirWatcher;

	ClientState m_ClientState;

	IpAddress m_ServerAddress;
	u16 m_ServerPort = 0;
public:
	Client(std::string path, IpAddress server_address, u16 server_port);

	virtual void OnDirChanged(FileAction action);

	void Run();
};

