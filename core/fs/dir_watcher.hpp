#ifndef CORTEX_DIR_WATCHER_HPP
#define CORTEX_DIR_WATCHER_HPP

#include <string>
#include <functional>
#include "fs/ignore_list.hpp"
#include "fs/dir.hpp"

using DirWatcherRef = std::unique_ptr<class DirWatcher>;

class DirWatcher {
public:
	using OnDirChangedCallback = std::function<void(FileAction)>;
public:
	virtual ~DirWatcher() = default;

	virtual bool WaitAndDispatchChanges() = 0;

	virtual bool AcknowledgedWriteEntireFile(const std::string &filepath, const void *data, size_t size) = 0;

	virtual bool AcknowledgedDeleteFile(const std::string &filepath) = 0;

	virtual bool AcknowledgedSetFileTime(const std::string &filepath, FileTime time) = 0;

	static DirWatcherRef Create(Dir *dir, OnDirChangedCallback callback);
};

#endif//CORTEX_DIR_WATCHER_HPP