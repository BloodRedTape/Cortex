#include "repository.hpp"
#include <iostream>
#include <chrono>
#include <thread>

Repository::Repository(std::string path):
	m_RepositoryPath(std::move(path)),
	m_DirWatcher(DirWatcher::Create(m_RepositoryPath.c_str(), std::bind(&Repository::OnDirChanged, this, std::placeholders::_1)))
{}

void Repository::OnDirChanged(FileAction action) {
	Commit commit{
		action,
		HashLastCommit()
	};

	std::cout << "NewCommit : " << commit.Previous << std::endl;
	std::cout << "\tActionType:   " << FileActionTypeString(commit.Action.Type) << std::endl;
	std::cout << "\tRelativePath: " << commit.Action.RelativeFilepath << std::endl;
	std::cout << "\tUnixTime:     " << commit.Action.ModificationTime.Seconds << std::endl;

	m_Commits.push_back(commit);
}

Hash Repository::HashLastCommit()
{
	if (!m_Commits.size()) 
		return Hash();

	const Commit &last = m_Commits.back();

	return Hash(last);
}

void Repository::Run() {
	while (true) {
		m_DirWatcher->DispatchChanges();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
