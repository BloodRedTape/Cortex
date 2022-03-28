#include "client_repository.hpp"
#include <iostream>
#include <chrono>
#include <thread>

ClientRepository::ClientRepository(std::string path):
	m_RepositoryDir(
		Dir::Create(std::move(path))
	),
	m_History(m_RepositoryDir->ReadEntireFile(".history").second),
	m_DirWatcher(
		DirWatcher::Create(
			m_RepositoryDir.get(),
			std::bind(&ClientRepository::OnDirChanged, this, std::placeholders::_1), 
			{std::regex(".history")},
			m_History.TraceDirState()
		)
	)
{}

ClientRepository::~ClientRepository(){
	std::string history = m_History.ToBinary();
	m_RepositoryDir->WriteEntireFile(".history", history.data(), history.size());
}

void ClientRepository::OnDirChanged(FileAction action){
	std::cout << "NewCommit : " << m_History.HashLastCommit() << std::endl;
	std::cout << "\tActionType:   " << FileActionTypeString(action.Type) << std::endl;
	std::cout << "\tRelativePath: " << action.RelativeFilepath << std::endl;
	std::cout << "\tUnixTime:     " << action.ModificationTime.Seconds << std::endl;

	m_History.Add(std::move(action));
}

void ClientRepository::Run(){
	for(int i = 0; i<3; i++){
		m_DirWatcher->DispatchChanges();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
//	m_History.SaveTo(m_RepositoryPath + ".history");
}
