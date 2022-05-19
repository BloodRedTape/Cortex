#pragma once

#include "commit.hpp"
#include "fs/dir_watcher.hpp"

constexpr const char *s_HistoryFilename = ".history";

class Repository {
private:
	DirRef m_RepositoryDir;
	CommitHistory m_History;
	DirWatcherRef m_DirWatcher;
public:
	Repository(std::string path);

	~Repository();

	virtual void OnDirChanged(FileAction action);

	void Run();
};

