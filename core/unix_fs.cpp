#include "fs/dir_watcher.hpp"
#include <fcntl.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>

using namespace std::literals::chrono_literals;

struct DirIt {
    DIR *Handle = nullptr;
    struct dirent64 *CurrentEntry;
    std::string DirectoryPath;

	DirIt(std::string dir_path):
        DirectoryPath(std::move(dir_path))
    {
        if(DirectoryPath.back() != '/')
            DirectoryPath.push_back('/');
        
        Handle = opendir(DirectoryPath.c_str());
		CurrentEntry = readdir64(Handle);
	}

	~DirIt() {
        closedir(Handle);
	}

	DirIt& operator++() {
		CurrentEntry = readdir64(Handle);
		return *this;
	}

	UnixTime ModifiedTime()const {
        std::string path = DirectoryPath + CurrentEntry->d_name;

        struct stat64 attr = {0};

        stat64(path.c_str(), &attr);
        
		return {(u64)attr.st_mtime};
	}

	const char* Name()const {
		return CurrentEntry->d_name;
	}

	operator bool()const {
		return IsValid();
	}

	bool IsValid()const {
		return CurrentEntry;
	}

	bool IsRegularDir()const {
		return IsValid() 
			&& CurrentEntry->d_type == DT_DIR
			&& !IsCurDir()
			&& !IsPrevDir();
	}

	bool IsCurDir()const {
		return IsValid() 
			&& CurrentEntry->d_type == DT_DIR
			&& CurrentEntry->d_name[0] == '.'
			&& CurrentEntry->d_name[1] == '\0';
	}

	bool IsPrevDir()const {
		return IsValid() 
			&& CurrentEntry->d_type == DT_DIR
			&& CurrentEntry->d_name[0] == '.'
			&& CurrentEntry->d_name[1] == '.'
			&& CurrentEntry->d_name[2] == '\0';
	}

	bool IsFile()const {
		return IsValid()                  
			&& CurrentEntry->d_type == DT_REG;
	}
};


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
        std::string path = m_DirPath + relative_path;

        if(!FileExists(path))
            return {false, {}};

        size_t size = 0;
        if(!FileSize(path, size))
            return {false, {}};

        int file = open(path.c_str(), O_RDONLY);
        std::string content(size, 0);

        if(read(file, content.data(), content.size()) != size){
            close(file);
            return {false, {}};
        }

        close(file);
		return {true, content};
	}

    static bool FileSize(const std::string &file_path, size_t &size){
        struct stat attr;   
        if(stat(file_path.c_str(), &attr) != 0)
            return false;

        size = attr.st_size;
        return true;
    }

	static bool DirExists(const std::string& dir_path){
        struct stat64 buffer;   
        return (stat64(dir_path.c_str(), &buffer) == 0) && ((buffer.st_mode & S_IFDIR) != 0); 
	}

	static bool FileExists(const std::string &file_path){ 
        struct stat64 buffer;   
        return (stat64(file_path.c_str(), &buffer) == 0) && ((buffer.st_mode & S_IFREG) != 0); 
	}

	bool CreateDirectories(std::string relative_path) {
        size_t dir_name_begin = 0;
		
		for (size_t i = dir_name_begin; i < relative_path.size(); i++) {
			if (relative_path[i] == '/') {
				relative_path[i] = 0;

				std::string full_subpath = m_DirPath + &relative_path[dir_name_begin];
				
				if(!DirExists(full_subpath))
                    if(mkdir(full_subpath.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
						return false;


				dir_name_begin = i + 1;
			}
		}
		return true;
	}

	bool WriteEntireFile(const std::string &relative_path, const void* data, size_t size)override{
        if(!CreateDirectories(relative_path))
            return false;

        std::string path = m_DirPath + relative_path;

        int file = open(path.c_str(), O_WRONLY);
        std::string content(size, 0);

        if(write(file, data, size) != size)
            return (close(file), false);

        close(file);
		return true;
	}

	bool DeleteFile(const std::string &relative_path)override{
        std::string path = m_DirPath + relative_path;
		return unlink(path.c_str()) == 0;
	}

	std::optional<FileTime> GetFileTime(const std::string& relative_path)override {
        std::string path = m_DirPath + relative_path;

        struct stat64 attr = {0};

        stat64(path.c_str(), &attr);
        
        return {
            FileTime{
                UnixTime{
                    (u64)attr.st_mtime
                },
                UnixTime{
                    (u64)attr.st_ctime
                }
            }
        };
	}

	bool SetFileTime(const std::string& relative_path, FileTime time)override {
        std::string path = m_DirPath + relative_path;

        struct stat64 attr;
        time_t mtime;
        struct utimbuf new_times;

        stat64(path.c_str(), &attr);

        new_times.actime = attr.st_atime; 
        new_times.modtime = (time_t)time.Modified.Seconds;
        utime(path.c_str(), &new_times);
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
