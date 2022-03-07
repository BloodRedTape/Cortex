#ifndef CORTEX_FILESYSTEM_HPP
#define CORTEX_FILESYSTEM_HPP

#include <memory>
#include <functional>
#include <string>
#include <cassert>
#include <ostream>
#include <istream>
#include <vector>
#include "time.hpp"

enum class FileActionType {
	Create = 0,
	Delete = 1,
	Update = 2
};

inline const char* FileActionTypeString(FileActionType type) {
	switch (type)
	{
	case FileActionType::Create: return "Create";
	case FileActionType::Delete: return "Delete";
	case FileActionType::Update: return "Update";
	}
	assert(false);
	return "Shit happens";
}

struct FileState {
	std::string RelativeFilepath;
	UnixTime ModificationTime;

	FileState(std::string rel_path, UnixTime time):
		RelativeFilepath(std::move(rel_path)),
		ModificationTime(time)
	{}
};

struct FileAction : FileState{
	FileActionType Type;

	FileAction(FileActionType type, UnixTime time, std::string relative_filepath):
		FileState(std::move(relative_filepath), time),
		Type(type)
	{}
};

struct DirState: std::vector<FileState> {
	friend std::ostream &operator<<(std::ostream &stream, const DirState &state);

	friend std::istream &operator>>(std::istream &stream, DirState &state);

	std::vector<FileAction> GetDiffFrom(const DirState &old);

	const FileState* Find(const FileState& other)const;
};

using DirWatcherRef = std::unique_ptr<class DirWatcher>;

class DirWatcher {
public:
	using OnDirChangedCallback = std::function<void(FileAction)>;
public:
	virtual ~DirWatcher() = default;

	virtual bool DispatchChanges() = 0;

	virtual DirState GetDirState() = 0;
	
	static DirWatcherRef Create(const char *dir_path, OnDirChangedCallback callback, bool is_blocking = true);
};

#endif //CORTEX_FILESYSTEM_HPP