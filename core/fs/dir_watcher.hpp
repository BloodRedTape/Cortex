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

	static DirWatcherRef Create(Dir *dir, OnDirChangedCallback callback, IgnoreList ignore_list = {});
};

#endif//CORTEX_DIR_WATCHER_HPP