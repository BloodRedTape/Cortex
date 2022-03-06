#ifndef CORTEX_REPOSITORY_HPP
#define CORTEX_REPOSITORY_HPP

#include <vector>
#include "commit.hpp"
#include "filesystem.hpp"

class Repository {
private:
	std::vector<Commit> m_Commits;
	std::string m_RepositoryPath;
	DirWatcherRef m_DirWatcher;
public:
	Repository(std::string path);

	void OnDirChanged(FileAction action);

	Hash HashLastCommit();
};

#endif//CORTEX_REPOSITORY_HPP