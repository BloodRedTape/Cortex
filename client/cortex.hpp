#pragma once

#include "utils.hpp"
#include "net/ip.hpp"
#include "commit.hpp"
#include "fs/dir_watcher.hpp"

constexpr const char *s_HistoryFilename= ".history";

class Client {
private:
	DirRef m_RepositoryDir;
	DirWatcherRef m_DirWatcher;

	CommitHistory m_History;
	FileActionAccumulator m_LocalChanges;

	IpAddress m_ServerAddress;
	u16 m_ServerPort = 0;
public:
	Client(std::string path, IpAddress server_address, u16 server_port);

	virtual void OnDirChanged(FileAction action);

	void Run();
private:
	void TryFlushLocalChanges();

	void TryPullRemoteChanges();
};

