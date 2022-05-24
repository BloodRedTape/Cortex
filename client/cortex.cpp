#include <iostream>
#include <sstream>
#include <optional>
#include "cortex.hpp"
#include "serializer.hpp"
#include "fs/dir.hpp"
#include "net/tcp_socket.hpp"
#include "protocol.hpp"
#include "error.hpp"


ClientState::ClientState(std::string binary){
	std::stringstream stream(binary);
	History = Serializer<CommitHistory>::Deserialize(stream);
	LocalChanges = Serializer<FileActionAccumulator>::Deserialize(stream);
	CurrentDirState = Serializer<DirState>::Deserialize(stream);
}

std::string ClientState::ToBinary() const{
	std::stringstream stream;

	Serializer<CommitHistory>::Serialize(stream, History);
	Serializer<FileActionAccumulator>::Serialize(stream, LocalChanges);
	Serializer<DirState>::Serialize(stream, CurrentDirState);

	return stream.str();
}

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
	m_ClientState(m_RepositoryDir->ReadEntireFile(s_StateFilename).second),
	m_DirWatcher(
		DirWatcher::Create(
			m_RepositoryDir.get(),
			std::bind(&Client::OnDirChanged, this, std::placeholders::_1), 
			{std::regex(s_StateFilename)}
		)
	),
	m_ServerAddress(server_address),
	m_ServerPort(server_port)
{}

void Client::OnDirChanged(FileAction action){
	Println("FileChanged: %", action.RelativeFilepath);

	m_ClientState.LocalChanges.Add(std::move(action));

	TryFlushLocalChanges();

	m_RepositoryDir->WriteEntireFile(s_StateFilename, m_ClientState.ToBinary());
}

void Client::TryFlushLocalChanges() {
	PushRequest push{
		m_ClientState.History.HashLastCommit(),
		m_ClientState.LocalChanges.ToVector(),
		CollectFilesData(m_RepositoryDir.get(), m_ClientState.LocalChanges)
	};

	auto resp = Transition(push, m_ServerAddress, m_ServerPort);

	if (!resp)
		return (void)Error("Transition failed, response did not happened");
	
	if (resp->Type == ResponceType::Success) {
		SuccessResponce success = resp->AsSuccessResponce();

		for(Commit commit: success.Commits){
			assert(commit.Previous == m_ClientState.History.HashLastCommit());
			m_ClientState.History.Add(std::move(commit));
		}

		m_ClientState.LocalChanges.Clear();
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
