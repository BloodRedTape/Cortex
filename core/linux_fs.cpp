#include "fs/dir_watcher.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std::literals::chrono_literals;

class LinuxDir: public Dir{
private:
	std::string m_DirPath;
public:
	LinuxDir(std::string path):
		m_DirPath(std::move(path))
	{}

	DirState GetDirState()override {
		return GetDirStateImpl(m_DirPath);
	}

	DirState GetDirStateImpl(const std::string &dir_path, std::string local_dir_path = "") {
		DirState state;
		return state;
	}

	std::pair<bool, std::string> ReadEntireFile(const std::string &relative_path)override{
		return {false, ""};
	}

	static bool DirExists(const std::string& dir_path){
		return false;
	}

	static bool FileExists(const std::string &file_path){ 
		return false;
	}

	bool CreateDirectories(std::string relative_path) {
		return false;
	}

	bool WriteEntireFile(const std::string &relative_path, const void* data, size_t size)override{
		return false;
	}

	bool DeleteFile(const std::string &relative_path)override{
		return false;
	}

	std::optional<FileTime> GetFileTime(const std::string& relative_path)override {
        return {};
	}

	bool SetFileTime(const std::string& relative_path, FileTime time)override {
		return false;
	}
};

DirRef Dir::Create(std::string dir_path) {
	return std::make_unique<LinuxDir>(dir_path);
}

class LinuxDirWatcher: public DirWatcher{
private:
	Dir *m_Dir = nullptr;
	OnDirChangedCallback m_Callback;
	DirState m_LastState;

	std::mutex m_Lock;
public:
	LinuxDirWatcher(Dir *dir, OnDirChangedCallback callback):
		m_Dir(dir),
		m_Callback(callback),
		m_LastState(m_Dir->GetDirState())
	{}

	bool WaitAndDispatchChanges()override{

		int diff_size = 0;
		for(;;){
			m_Lock.lock();
			DirState current_state = m_Dir->GetDirState();
			DirStateDiff diff = current_state.GetDiffFrom(m_LastState);
			m_Lock.unlock();
			diff_size = diff.size();

			if(!diff_size){
				std::this_thread::sleep_for(1s);
				continue;
			}

			for (const FileAction& action : diff)
				m_Callback(action);
			
			m_Lock.lock();
			m_LastState = std::move(current_state);
			m_Lock.unlock();
		}while(!diff_size);

		return true;
	}

	bool AcknowledgedWriteEntireFile(const std::string& filepath, const void* data, size_t size)override{
		std::unique_lock<std::mutex> guard(m_Lock);

		bool is_created = !m_LastState.Has(filepath);

		if(!m_Dir->WriteEntireFile(filepath, data, size))
			return false;

		if(is_created)
			m_LastState.push_back({filepath, UnixTime{0}});

		FileMeta *file = m_LastState.Find(filepath);

		file->ModificationTime = m_Dir->GetFileTime(filepath)->Modified;

		return true;
	}

	bool AcknowledgedDeleteFile(const std::string& filepath)override{
		std::unique_lock<std::mutex> guard(m_Lock);

		m_LastState.Remove(m_LastState.Find(filepath));
		return m_Dir->DeleteFile(filepath);
	}

	bool AcknowledgedSetFileTime(const std::string& filepath, FileTime time)override{
		std::unique_lock<std::mutex> guard(m_Lock);

		FileMeta *file = m_LastState.Find(filepath);
		if(!file)
			return false;

		file->ModificationTime = time.Modified;

		return m_Dir->SetFileTime(filepath, time);
	}
};

DirWatcherRef DirWatcher::Create(Dir *dir, OnDirChangedCallback callback) {
	return std::make_unique<LinuxDirWatcher>(dir, callback);
}
