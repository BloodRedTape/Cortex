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

std::string WstrToUtf8Str(const std::wstring& wstr){

	if (wstr.empty())
		return {};

	int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), nullptr, 0, nullptr, nullptr);

    if (sizeRequired <= 0)
		return {};

	std::string retStr(sizeRequired, '\0');

	int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), retStr.data(), retStr.size(), nullptr, nullptr);

	if(bytesConverted <= 0)
		Error("Can't convert string to UTF-8");

	for (char& ch : retStr)
		if(ch == '\\')
			ch = '/';

	return retStr;
}

std::wstring Utf8ToWstr(const std::string &str){

	if (str.empty())
		return {};


	int sizeRequired = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), nullptr, 0);

    if (sizeRequired <= 0)
		return {};


	std::wstring retStr(sizeRequired, L'\0');

	int bytesConverted = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), retStr.data(), retStr.size());

	if(bytesConverted <= 0)
		Error("Can't convert string to UTF-16");

	for (wchar_t& ch : retStr)
		if(ch == L'/')
			ch = L'\\';

	return retStr;
}

struct DirIt {
	HANDLE CurrentFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAW CurrentFileMetadata = {};

	DirIt(const std::wstring &dir_path) {
		std::wstring path = dir_path;
		if(path.back() != L'\\')
			path.push_back(L'\\');
		path.push_back(L'*');
		CurrentFile = FindFirstFileW(path.c_str(), &CurrentFileMetadata);
	}

	~DirIt() {
		if(IsValid())
			FindClose(CurrentFile);
	}

	DirIt& operator++() {
		if(!FindNextFileW(CurrentFile, &CurrentFileMetadata)){
			FindClose(CurrentFile);
			CurrentFile = INVALID_HANDLE_VALUE;
		}
		return *this;
	}

	UnixTime ModifiedTime()const {
		return {WindowsFileTimeToUnixSeconds(CurrentFileMetadata.ftLastWriteTime)};
	}

	std::wstring Name()const {
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
	std::wstring m_DirPath;
public:
	Win32Dir(std::string path):
		m_DirPath(Utf8ToWstr(path))
	{}

	DirState GetDirState()override {
		return GetDirStateImpl(m_DirPath);
	}

	DirState GetDirStateImpl(const std::wstring &dir_path, std::wstring local_dir_path = L"") {
		DirState state;

		for (DirIt it(dir_path); it; ++it) {
			std::wstring name = it.Name();
			if(it.IsFile())
				state.emplace_back(WstrToUtf8Str(local_dir_path + name), it.ModifiedTime());
			if (it.IsRegularDir()) {
				auto sub_state = GetDirStateImpl(dir_path + name + L"\\", local_dir_path + name + L"\\");
				state.reserve(state.size() + sub_state.size());
				std::move(std::begin(sub_state), std::end(sub_state), std::back_inserter(state));
			}
		}

		return state;
	}

	std::pair<bool, std::string> ReadEntireFile(const std::string &relative_path)override{
		std::wstring path = m_DirPath + Utf8ToWstr(relative_path);

		HANDLE file = (HANDLE)CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, 0);
		if(file == INVALID_HANDLE_VALUE)
			return {Error("ReadEntireFile(%): OpenFile: %", relative_path, GetLastError()), {}};
		
		LARGE_INTEGER size = {};		

		if(!GetFileSizeEx(file, &size) || std::numeric_limits<DWORD>::max() < size.QuadPart){
			CloseHandle(file);
			return {Error("ReadEntireFile(%): GetFileSize: %", relative_path, GetLastError()), {}};
		}
			
		std::string content;

		content.resize(size.LowPart);

		if(!ReadFile(file, &content[0], size.LowPart, nullptr, nullptr)){
			CloseHandle(file);
			return {Error("ReadEntireFile(%): ReadFile: %", relative_path, GetLastError()), {}};
		}
		
		CloseHandle(file);
		return {true, content};
	}

	static bool DirExists(const std::wstring& dir_path)
	{
		DWORD ftyp = GetFileAttributesW(dir_path.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;

		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;

		return false;
	}

	static bool FileExists(const std::wstring &file_path)
	{ 
		DWORD dwAttrib = GetFileAttributesW(file_path.c_str());

		return  (dwAttrib != INVALID_FILE_ATTRIBUTES
			&& !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	bool CreateDirectories(std::string relative_path) {
		size_t dir_name_begin = 0;
		
		for (size_t i = dir_name_begin; i < relative_path.size(); i++) {
			if (relative_path[i] == '/') {
				relative_path[i] = 0;

				std::wstring full_subpath = m_DirPath + Utf8ToWstr(&relative_path[dir_name_begin]);
				
				if(!DirExists(full_subpath))
					if(!CreateDirectoryW(full_subpath.c_str(), nullptr))
						return false;

				dir_name_begin = i + 1;
			}
		}
		return true;
	}

	bool WriteEntireFile(const std::string &relative_path, const void* data, size_t size)override{
		if(!CreateDirectories(relative_path))
			return Error("WriteEntireFile(%): Can't create subdirectories tree", relative_path);

		std::wstring path = m_DirPath + Utf8ToWstr(relative_path);

		HANDLE file = (HANDLE)CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, 0, 0);
		if(file == INVALID_HANDLE_VALUE){
			return Error("WriteEntireFile(%): OpenFile: %", relative_path, GetLastError());
		}

		if(!WriteFile(file, data, size, nullptr, nullptr)){
			CloseHandle(file);
			return Error("WriteEntireFile(%): WriteFile: %", relative_path, GetLastError());
		}
		CloseHandle(file);
		return true;
	}

	#undef DeleteFile
	bool DeleteFile(const std::string &relative_path)override{
		std::wstring path = m_DirPath + Utf8ToWstr(relative_path);
		if(!DeleteFileW(path.c_str()))
			return Error("DeleteFile: %", GetLastError());
		return true;
	}

	std::optional<FileTime> GetFileTime(const std::string& relative_path)override {
		std::wstring path = m_DirPath + Utf8ToWstr(relative_path);

		if(!FileExists(path))
			return {};

		HANDLE file = CreateFileW(path.c_str(), FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, 0, 0);

		if(file == INVALID_HANDLE_VALUE)
			return {};

		FILETIME created, modified;
		auto res = ::GetFileTime(file, &created, nullptr, &modified);

		CloseHandle(file);

		if(!res)
			return {};


		
		return {
			FileTime{
				UnixTime{WindowsFileTimeToUnixSeconds(created)},
				UnixTime{WindowsFileTimeToUnixSeconds(modified)}
			}
		};
	}

	bool SetFileTime(const std::string& relative_path, FileTime time)override {
		std::wstring path = m_DirPath + Utf8ToWstr(relative_path);
		if(!FileExists(path))
			return false;

		HANDLE file = CreateFileW(path.c_str(), FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, 0, 0);

		if(file == INVALID_HANDLE_VALUE)
			return {};
		
		FILETIME created = UnixSecondsToWindowsFileTime(time.Created.Seconds);
		FILETIME modified = UnixSecondsToWindowsFileTime(time.Modified.Seconds);

		auto result = ::SetFileTime(file, &created, nullptr, &modified);
		CloseHandle(file);
		return result;
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
