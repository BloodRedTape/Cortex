#include "repository.hpp"
#include <iostream>

void Repository::OnDirChanged(FileAction action) {
	Commit commit{
		action,
		HashLastCommit()
	};

	Commits.push_back(commit);

	std::cout << "NewCommit : " << commit.Previous << std::endl;
	std::cout << "\tActionType:   " << FileActionTypeString(commit.Action.Type) << std::endl;
	std::cout << "\tRelativePath: " << commit.Action.RelativeFilepath << std::endl;
	std::cout << "\tUnixTime:     " << commit.Action.Time.Seconds << std::endl;
}

Hash Repository::HashLastCommit()
{
	if (!Commits.size()) 
		return Hash();

	const Commit &last = Commits.back();

	return Hash(last);
}
