#ifndef CORTEX_CLIENT_REPOSITORY_HPP
#define CORTEX_CLIENT_REPOSITORY_HPP

#include <vector>
#include "commit.hpp"
#include "filesystem.hpp"

constexpr const char *s_HistoryFilename = ".history";

class ClientRepository {
private:
private:
	CommitHistory m_History;
	std::string m_RepositoryPath;
	DirWatcherRef m_DirWatcher;
public:
	ClientRepository(std::string path);

	~ClientRepository();

	void OnDirChanged(FileAction action);

	void Run();
};

#endif//CORTEX_CLIENT_REPOSITORY_HPP