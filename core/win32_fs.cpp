#include "fs/dir_watcher.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std::literals::chrono_literals;

#undef max

#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL

u64 WindowsTickToUnixSeconds(u64 windowsTicks) {
     return (u64)(windowsTicks / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);
}

u64 UnixSecondsToWindowsTicks(u64 unix_seconds) {
     return (u64)((unix_seconds + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK);
}

u64 WindowsFileTimeToUnixSeconds(const FILETIME &filetime) {
	ULARGE_INTEGER time = {};

	time.LowPart = filetime.dwLowDateTime;
	time.HighPart= filetime.dwHighDateTime;

	return {WindowsTickToUnixSeconds(time.QuadPart)};
}

FILETIME UnixSecondsToWindowsFileTime(u64 unix_seconds) {
	LARGE_INTEGER time;
	time.QuadPart = UnixSecondsToWindowsTicks(unix_seconds);
	
	FILETIME result;
	result.dwHighDateTime = time.HighPart;
	result.dwLowDateTime = time.LowPart;
	return result;
}

struct DirIt {
	HANDLE CurrentFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA CurrentFileMetadata = {};

	DirIt(const std::string &dir_path) {
		std::string path = dir_path;
		if(path.back() != '\\')
			path.push_back('\\');
		path.push_back('*');
		CurrentFile = FindFirstFileA(path.c_str(), &CurrentFileMetadata);
	}

	~DirIt() {
		if(IsValid())
			FindClose(CurrentFile);
	}

	DirIt& operator++() {
		if(!FindNextFileA(CurrentFile, &CurrentFileMetadata)){
			FindClose(CurrentFile);
			CurrentFile = INVALID_HANDLE_VALUE;
		}
		return *this;
	}

	UnixTime ModifiedTime()const {
		return {WindowsFileTimeToUnixSeconds(CurrentFileMetadata.ftLastWriteTime)};
	}

	const char* Name()const {
		return CurrentFileMetadata.cFileName;
	}

	operator bool()const {
		return IsValid();
	}

	bool IsValid()const {
		return CurrentFile != INVALID_HANDLE_VALUE;
	}

	bool IsRegularDir()const {
		return IsValid() 
			&& CurrentFileMetadata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			&& !IsCurDir()
			&& !IsPrevDir();
	}

	bool IsCurDir()const {
		return IsValid() 
			&& CurrentFileMetadata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			&& CurrentFileMetadata.cFileName[0] == '.'
			&& CurrentFileMetadata.cFileName[1] == '\0';
	}

	bool IsPrevDir()const {
		return IsValid() 
			&& CurrentFileMetadata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			&& CurrentFileMetadata.cFileName[0] == '.'
			&& CurrentFileMetadata.cFileName[1] == '.'
			&& CurrentFileMetadata.cFileName[2] == '\0';
	}

	bool IsFile()const {
		return IsValid() 
			&& !IsRegularDir()
			&& !IsCurDir()
			&& !IsPrevDir();
	}
};

class Win32Dir: public Dir{
private:
	std::string m_DirPath;
public:
	Win32Dir(std::string path):
		m_DirPath(std::move(path))
	{}

	DirState GetDirState()override {
		return GetDirStateImpl(m_DirPath);
	}

	DirState GetDirStateImpl(const std::string &dir_path, std::string local_dir_path = "") {
		DirState state;

		for (DirIt it(dir_path); it; ++it) {
			if(it.IsFile())
				state.emplace_back(local_dir_path + it.Name(), it.ModifiedTime());
			if (it.IsRegularDir()) {
				auto sub_state = GetDirStateImpl(dir_path + it.Name() + "\\", local_dir_path + it.Name() + "\\");
				state.reserve(state.size() + sub_state.size());
				std::move(std::begin(sub_state), std::end(sub_state), std::back_inserter(state));
			}
		}

		return state;
	}

	std::pair<bool, std::string> ReadEntireFile(const std::string &relative_path)override{
		OFSTRUCT fo = {sizeof(fo)};
		HANDLE file = (HANDLE)OpenFile((m_DirPath + relative_path).c_str(), &fo, 0);
		if(!file)
			return {false, {}};
		LARGE_INTEGER size = {};		

		if(!GetFileSizeEx(file, &size) || std::numeric_limits<DWORD>::max() < size.QuadPart){
			CloseHandle(file);
			return {false, {}};
		}
			
		std::string content;

		content.resize(size.LowPart);

		if(!ReadFile(file, &content[0], size.LowPart, nullptr, nullptr)){
			CloseHandle(file);
			return {false, {}};
		}
		
		CloseHandle(file);
		return {true, content};
	}

	static bool DirExists(const std::string& dir_path)
	{
		DWORD ftyp = GetFileAttributesA(dir_path.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;

		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;

		return false;
	}

	static bool FileExists(const std::string &file_path)
	{ 
		DWORD dwAttrib = GetFileAttributesA(file_path.c_str());

		return  (dwAttrib != INVALID_FILE_ATTRIBUTES
			&& !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	bool CreateDirectories(std::string relative_path) {
		size_t dir_name_begin = 0;
		
		for (size_t i = dir_name_begin; i < relative_path.size(); i++) {
			if (relative_path[i] == '\\') {
				relative_path[i] = 0;

				std::string full_subpath = m_DirPath + &relative_path[dir_name_begin];
				
				if(!DirExists(full_subpath))
					if(!CreateDirectoryA(full_subpath.c_str(), nullptr))
						return false;

				dir_name_begin = i + 1;
			}
		}
		return true;
	}

	bool WriteEntireFile(const std::string &relative_path, const void* data, size_t size)override{
		if(!CreateDirectories(relative_path))
			return false;

		OFSTRUCT fo = {sizeof(fo)};
		HANDLE file = (HANDLE)OpenFile((m_DirPath + relative_path).c_str(), &fo, OF_CREATE);
		if(!file)
			return false;

		if(!WriteFile(file, data, size, nullptr, nullptr)){
			CloseHandle(file);
			return false;
		}
		CloseHandle(file);
		return true;
	}

	#undef DeleteFile
	bool DeleteFile(const std::string &relative_path)override{
		return DeleteFileA(relative_path.c_str()) != 0;
	}

	std::optional<FileTime> GetFileTime(const std::string& relative_path)override {
		std::string path = m_DirPath + relative_path;

		if(!FileExists(path.c_str()))
			return {};

		HANDLE file = CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, 0, 0);

		if(file == INVALID_HANDLE_VALUE)
			return {};

		FILETIME created, modified;
		if(!::GetFileTime(file, &created, nullptr, &modified))
			return {};

		
		return {
			FileTime{
				UnixTime{WindowsFileTimeToUnixSeconds(created)},
				UnixTime{WindowsFileTimeToUnixSeconds(modified)}
			}
		};
	}

	bool SetFileTime(const std::string& relative_path, FileTime time)override {
		std::string path = m_DirPath + relative_path;
		if(!FileExists(path.c_str()))
			return false;

		HANDLE file = CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, 0, 0);

		if(file == INVALID_HANDLE_VALUE)
			return {};
		
		FILETIME created = UnixSecondsToWindowsFileTime(time.Created.Seconds);
		FILETIME modified = UnixSecondsToWindowsFileTime(time.Modified.Seconds);

		return ::SetFileTime(file, &created, nullptr, &modified);
	}
};

DirRef Dir::Create(std::string dir_path) {
	return std::make_unique<Win32Dir>(dir_path);
}

class Win32DirWatcher: public DirWatcher{
private:
	Dir *m_Dir = nullptr;
	OnDirChangedCallback m_Callback;
	DirState m_LastState;

	std::mutex m_Lock;
public:
	Win32DirWatcher(Dir *dir, OnDirChangedCallback callback):
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

	bool AcknowledgedWriteEntireFile(const std::string& filepath, const void* data, size_t size) {
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

	bool AcknowledgedDeleteFile(const std::string& filepath) {
		std::unique_lock<std::mutex> guard(m_Lock);

		m_LastState.Remove(m_LastState.Find(filepath));
		return m_Dir->DeleteFile(filepath);
	}

	bool AcknowledgedSetFileTime(const std::string& filepath, FileTime time) {
		std::unique_lock<std::mutex> guard(m_Lock);

		FileMeta *file = m_LastState.Find(filepath);
		if(!file)
			return false;

		file->ModificationTime = time.Modified;

		return m_Dir->SetFileTime(filepath, time);
	}
};



DirWatcherRef DirWatcher::Create(Dir *dir, OnDirChangedCallback callback) {
	return std::make_unique<Win32DirWatcher>(dir, callback);
}
