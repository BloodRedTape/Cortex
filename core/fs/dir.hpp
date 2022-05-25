#ifndef CORTEX_DIR_HPP
#define CORTEX_DIR_HPP

#include <memory>
#include <optional>
#include "fs/dir_state.hpp"

using DirRef = std::unique_ptr<class Dir>;

struct FileTime {
	UnixTime Created;
	UnixTime Modified;
};

class Dir {
public:
	virtual ~Dir() = default;

	virtual DirState GetDirState() = 0;

	virtual std::pair<bool, std::string> ReadEntireFile(const std::string &relative_path) = 0;

	virtual bool WriteEntireFile(const std::string &relative_path, const void *data, size_t size) = 0;

	bool WriteEntireFile(const std::string &relative_path, const std::string &content) {
		return WriteEntireFile(relative_path, content.data(), content.size());
	}

	virtual std::optional<FileTime> GetFileTime(const std::string &relative_path) = 0;

	virtual bool SetFileTime(const std::string &relative_path, FileTime time) = 0;

	virtual bool DeleteFile(const std::string &relative_path) = 0;

	static DirRef Create(std::string dir_path);
};

#endif//CORTEX_DIR_HPP