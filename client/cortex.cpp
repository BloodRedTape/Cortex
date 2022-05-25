#include <iostream>
#include <sstream>
#include <optional>
#include "cortex.hpp"
#include "serializer.hpp"
#include "fs/dir.hpp"
#include "net/tcp_socket.hpp"
#include "net/udp_socket.hpp"
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
	m_IgnoreList({std::regex(s_HistoryFilename)}),
	m_RepositoryDir(
		Dir::Create(std::move(path))
	),
	m_History(m_RepositoryDir->ReadEntireFile(s_HistoryFilename).second),
	m_ServerAddress(server_address),
	m_ServerPort(server_port)
{

	for(const FileAction &action: m_RepositoryDir->GetDirState().GetDiffFrom(m_History.TraceDirState()))
		OnDirChanged(action);

	m_DirWatcher = DirWatcher::Create(m_RepositoryDir.get(), [this](FileAction action) {
		m_Pipe.PushEvent(action);
	});

	m_DirWatcherThread = std::thread([this]() {
		while (m_IsRunning) {
			m_DirWatcher->WaitAndDispatchChanges();
		}
	});

	m_Pipe.RegisterEventType<FileAction>(std::bind(&Client::OnDirChanged, this, std::placeholders::_1));

	m_BroadcastThread = std::thread([this]() {
		UdpSocket socket;
		if (!socket.Bind(IpAddress::Any, BroadcastListenPort)) {
			m_IsRunning = false;
			return Error("Can't bind to port '%' for broadcast listening", BroadcastListenPort);
		}

		while (m_IsRunning) {
			BroadcastProtocolHeader header;
			if (socket.Receive(&header, sizeof(header)) != sizeof(header)) {
				Println("BroadcastThread: partial received");
				continue;
			}

			if (header.MagicWord != BroadcastMagicWord) {
				Println("BroadcastThread: broken broadcast header");
				continue;
			}
			
			m_Pipe.PushEvent(BroadcastEvent{});
		}
	});

	m_Pipe.RegisterEventType<BroadcastEvent>(std::bind(&Client::OnBroadcastEvent, this, std::placeholders::_1));
}

Client::~Client(){
	m_DirWatcherThread.join();
	m_BroadcastThread.join();
}

void Client::OnDirChanged(FileAction action){
	if(m_IgnoreList.ShouldBeIgnored(action.RelativeFilepath))
		return;

	Println("FileChanged: %", action.RelativeFilepath);

	m_LocalChanges.Add(std::move(action));

	TryFlushLocalChanges();
}

void Client::OnBroadcastEvent(BroadcastEvent){
	TryPullRemoteChanges();
	TryFlushLocalChanges();
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
			
		m_History.Add(success.Commits);

		m_LocalChanges.Clear();
	} 
	if (resp->Type == ResponceType::Diff) {
		ApplyDiff(resp->AsDiffResponce());
	}
	m_RepositoryDir->WriteEntireFile(s_HistoryFilename, m_History.ToBinary());

}

void Client::TryPullRemoteChanges() {
	PullRequest pull{
		m_History.HashLastCommit()
	};

	auto resp = Transition(pull, m_ServerAddress, m_ServerPort);

	if (!resp)
		return (void)Error("Transition failed, response did not happened");

	if (resp->Type == ResponceType::Diff) 
		ApplyDiff(resp->AsDiffResponce());
	else 
		assert(false);
	m_RepositoryDir->WriteEntireFile(s_HistoryFilename, m_History.ToBinary());
}

void Client::ApplyDiff(const DiffResponce& diff){

	DirState state = m_History.TraceDirState();

    for (const FileData& file : diff.ResultingFiles)
        m_DirWatcher->AcknowledgedWriteEntireFile(file.RelativeFilepath, file.Content.data(), file.Content.size());

    for (const Commit &commit: diff.Commits){
        const FileAction &action = commit.Action;
        if(action.Type == FileActionType::Delete) 
            //XXX: Handle failure
            m_DirWatcher->AcknowledgedDeleteFile(commit.Action.RelativeFilepath);
        if (action.Type == FileActionType::Write) {
            if (state.Has(action.RelativeFilepath)) {
                m_DirWatcher->AcknowledgedSetFileTime(action.RelativeFilepath, {m_RepositoryDir->GetFileTime(action.RelativeFilepath)->Created, action.Time});
            } else {
                m_DirWatcher->AcknowledgedSetFileTime(action.RelativeFilepath, {action.Time, action.Time});
            }
        }
    }

    m_History.Add(diff.Commits);

    for(const Commit &commit: diff.Commits)
        m_LocalChanges.OverrideIfConflicted(commit.Action);	
}

void Client::Run(){
	while(m_IsRunning) {
		m_Pipe.WaitAndDispath();	
	}
}

int main(int argc, char **argv) {
	Client("W:\\Dev\\Cortex\\out\\client\\", IpAddress::Loopback, 25565).Run();
}
