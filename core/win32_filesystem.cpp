#include "filesystem.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>

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

class Win32DirWatcher: public DirWatcher{
private:
	std::string m_DirPath;
	OnDirChangedCallback m_Callback;
	DirState m_LastState;
	bool m_IsBlocking;
public:
	Win32DirWatcher(std::string dir_path, OnDirChangedCallback callback, DirState initial_state, bool is_blocking):
		m_DirPath(std::move(dir_path)),
		m_Callback(callback),
		m_LastState(std::move(initial_state)),
		m_IsBlocking(is_blocking)
	{
#if 0
		auto dir_state = GetDirState();
		
		for (const FileState& file : dir_state) {
			std::cout << file.RelativeFilepath << std::endl;
			std::cout << '\t' << file.ModificationTime.Seconds << std::endl;
		}
#endif
	}

	bool DispatchChanges()override{
		DirState current_state = GetDirState();

		DirStateDiff diff = current_state.GetDiffFrom(m_LastState);

		if (!diff.size()) 
			return false;

		for (const FileAction& action : diff) 
			m_Callback(action);

		m_LastState = std::move(current_state);

		return true;
	}

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
};

DirWatcherRef DirWatcher::Create(std::string dir_path, OnDirChangedCallback callback, DirState initial_state, bool is_blocking) {
	return std::make_unique<Win32DirWatcher>(std::move(dir_path), callback, std::move(initial_state), is_blocking);
}
