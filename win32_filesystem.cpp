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
	OnDirChangedCallback m_Callback;
	bool m_IsBlocking;
	std::string m_DirPath;
public:
	Win32DirWatcher(std::string dir_path, OnDirChangedCallback callback, bool is_blocking):
		m_Callback(callback),
		m_IsBlocking(is_blocking),
		m_DirPath(std::move(dir_path))
	{
		auto dir_state = GetDirState();
		
		for (const FileState& file : dir_state) {
			std::cout << file.RelativeFilepath << std::endl;
			std::cout << '\t' << file.ModificationTime.Seconds << std::endl;
		}

	}

	bool DispatchChanges()override{
		return false;
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

DirWatcherRef DirWatcher::Create(std::string dir_path, OnDirChangedCallback callback, bool is_blocking) {
	return std::make_unique<Win32DirWatcher>(std::move(dir_path), callback, is_blocking);
}
