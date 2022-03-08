#include "repository.hpp"
#include <iostream>
#include <chrono>
#include <thread>

Repository::Repository(std::string path):
	m_RepositoryPath(std::move(path)),
	m_DirWatcher(DirWatcher::Create(m_RepositoryPath.c_str(), std::bind(&Repository::OnDirChanged, this, std::placeholders::_1)))
{}

void Repository::OnDirChanged(FileAction action) {
	std::cout << "NewCommit : " << m_History.HashLastCommit() << std::endl;
	std::cout << "\tActionType:   " << FileActionTypeString(action.Type) << std::endl;
	std::cout << "\tRelativePath: " << action.RelativeFilepath << std::endl;
	std::cout << "\tUnixTime:     " << action.ModificationTime.Seconds << std::endl;

	m_History.Add(std::move(action));
}

void Repository::Run() {
	for(int i = 0; i<5; i++){
		m_DirWatcher->DispatchChanges();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	m_History.SaveTo(m_RepositoryPath + ".history");
}
