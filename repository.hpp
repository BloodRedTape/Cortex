#ifndef CORTEX_REPOSITORY_HPP
#define CORTEX_REPOSITORY_HPP

#include <vector>
#include "commit.hpp"
#include "filesystem.hpp"

class Repository {
private:
	static constexpr const char *s_HistoryFilename = ".history";
private:
	CommitHistory m_History;
	std::string m_RepositoryPath;
	DirWatcherRef m_DirWatcher;
public:
	Repository(std::string path);

	~Repository();

	void OnDirChanged(FileAction action);

	void Run();
};

class ClientRepository: public Repository {
private:
public:
	ClientRepository(std::string path):
		Repository(std::move(path))
	{}
};
class ServerRepository: public Repository {
private:
public:
	ServerRepository(std::string path):
		Repository(std::move(path))
	{}
};

#endif//CORTEX_REPOSITORY_HPP