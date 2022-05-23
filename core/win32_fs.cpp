#include "fs/dir_watcher.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>

#undef max

#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL

u64 WindowsTickToUnixSeconds(u64 windowsTicks) {
     return (u64)(windowsTicks / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);
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
		ULARGE_INTEGER time = {};

		time.LowPart = CurrentFileMetadata.ftLastWriteTime.dwLowDateTime;
		time.HighPart= CurrentFileMetadata.ftLastWriteTime.dwHighDateTime;

		return {WindowsTickToUnixSeconds(time.QuadPart)};
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

	bool WriteEntireFile(const std::string &relative_path, const void* data, size_t size)override{
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
};

DirRef Dir::Create(std::string dir_path) {
	return std::make_unique<Win32Dir>(dir_path);
}

class Win32DirWatcher: public DirWatcher{
private:
	Dir *m_Dir = nullptr;
	OnDirChangedCallback m_Callback;
	IgnoreList m_IgnoreList;
	DirState m_LastState;
	bool m_IsBlocking;
public:
	Win32DirWatcher(Dir *dir, OnDirChangedCallback callback, IgnoreList ignore_list, DirState initial_state, bool is_blocking):
		m_Dir(dir),
		m_Callback(callback),
		m_IgnoreList(std::move(ignore_list)),
		m_LastState(std::move(initial_state)),
		m_IsBlocking(is_blocking)
	{
#if 0
		auto dir_state = GetDirState();
		
		for (const FileMeta& file : dir_state) {
			std::cout << file.RelativeFilepath << std::endl;
			std::cout << '\t' << file.ModificationTime.Seconds << std::endl;
		}
#endif
	}

	bool DispatchChanges()override{
		DirState current_state = m_Dir->GetDirState();

		DirStateDiff diff = current_state.GetDiffFrom(m_LastState);

		if (!diff.size()) 
			return false;

		for (const FileAction& action : diff){
			if(!m_IgnoreList.ShouldBeIgnored(action.RelativeFilepath))
				m_Callback(action);
		}

		m_LastState = std::move(current_state);

		return true;
	}


};



DirWatcherRef DirWatcher::Create(Dir *dir, OnDirChangedCallback callback, IgnoreList ignore_list, DirState initial_state, bool is_blocking) {
	return std::make_unique<Win32DirWatcher>(dir, callback, std::move(ignore_list), std::move(initial_state), is_blocking);
}
