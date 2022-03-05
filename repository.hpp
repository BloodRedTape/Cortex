#ifndef CORTEX_REPOSITORY_HPP
#define CORTEX_REPOSITORY_HPP

#include <vector>
#include "commit.hpp"
#include "filesystem.hpp"

struct Repository {
	std::vector<Commit> Commits;
	std::string RepositoryPath;

	void OnDirChanged(FileAction action);

	Hash HashLastCommit();
};

#endif//CORTEX_REPOSITORY_HPP