#ifndef CORTEX_CLIENT_REPOSITORY_HPP
#define CORTEX_CLIENT_REPOSITORY_HPP

#include <vector>
#include "commit.hpp"
#include "fs/dir_watcher.hpp"

constexpr const char *s_HistoryFilename = ".history";

class ClientRepository {
private:
private:
	DirRef m_RepositoryDir;
	CommitHistory m_History;
	DirWatcherRef m_DirWatcher;
public:
	ClientRepository(std::string path);

	~ClientRepository();

	void OnDirChanged(FileAction action);

	void Run();
};

#endif//CORTEX_CLIENT_REPOSITORY_HPP