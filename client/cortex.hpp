#pragma once

#include <atomic>
#include "utils.hpp"
#include "net/ip.hpp"
#include "commit.hpp"
#include "fs/dir_watcher.hpp"
#include "async/event.hpp"
#include "protocol.hpp"

constexpr const char *s_HistoryFilename= ".history";

struct BroadcastEvent{ };

class Client {
private:
	const IgnoreList m_IgnoreList;
	std::atomic<bool> m_IsRunning{true};
	EventPipe m_Pipe;
	DirRef m_RepositoryDir;

	DirWatcherRef m_DirWatcher;

	std::thread m_DirWatcherThread;
	std::thread m_BroadcastThread;

	CommitHistory m_History;
	FileActionAccumulator m_LocalChanges;

	IpAddress m_ServerAddress;
	u16 m_ServerPort = 0;
public:
	Client(std::string path, IpAddress server_address, u16 server_port);

	~Client();

	void OnDirChanged(FileAction action);

	void OnBroadcastEvent(BroadcastEvent);

	void Run();
private:
	void TryFlushLocalChanges();

	void TryPullRemoteChanges();
	
	void ApplyDiff(const DiffResponce &diff);
};

