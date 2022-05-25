#include <iostream>
#include <sstream>
#include <optional>
#include "cortex.hpp"
#include "serializer.hpp"
#include "fs/dir.hpp"
#include "net/tcp_socket.hpp"
#include "protocol.hpp"
#include "error.hpp"

std::optional<Responce> Transition(const Request &req, IpAddress address, u16 port) {
	TcpSocket socket;
	if(!socket.Connect(address, port))
		return {};
	
	if (!req.Send(socket))
		return Error("Disconnected during request sending"), std::optional<Responce>();

	std::optional<Responce> resp = Responce::Receive(socket);

	if(!resp)
		return Error("Responce failed"), std::optional<Responce>();
	
	return resp;
}


Client::Client(std::string path, IpAddress server_address, u16 server_port):
	m_RepositoryDir(
		Dir::Create(std::move(path))
	),
	m_History(m_RepositoryDir->ReadEntireFile(s_HistoryFilename).second),
	m_DirWatcher(
		DirWatcher::Create(
			m_RepositoryDir.get(),
			std::bind(&Client::OnDirChanged, this, std::placeholders::_1), 
			{std::regex(s_HistoryFilename)}
		)
	),
	m_ServerAddress(server_address),
	m_ServerPort(server_port)
{}

void Client::OnDirChanged(FileAction action){
	Println("FileChanged: %", action.RelativeFilepath);

	m_LocalChanges.Add(std::move(action));

	TryFlushLocalChanges();

	m_RepositoryDir->WriteEntireFile(s_HistoryFilename, m_History.ToBinary());
}

void Client::TryFlushLocalChanges() {
	PushRequest push{
		m_History.HashLastCommit(),
		m_LocalChanges.ToVector(),
		CollectFilesData(m_RepositoryDir.get(), m_LocalChanges)
	};

	auto resp = Transition(push, m_ServerAddress, m_ServerPort);

	if (!resp)
		return (void)Error("Transition failed, response did not happened");
	
	if (resp->Type == ResponceType::Success) {
		SuccessResponce success = resp->AsSuccessResponce();

		for(Commit commit: success.Commits){
			assert(commit.Previous == m_History.HashLastCommit());
			m_History.Add(std::move(commit));
		}

		m_LocalChanges.Clear();
	} else {
		assert(false);
	}

}

void Client::Run(){
	while(true) {
		m_DirWatcher->WaitAndDispatchChanges();
	}
}

int main() {
	Client("W:\\Dev\\Cortex\\out\\client\\", IpAddress::Loopback, 25565).Run();
}
