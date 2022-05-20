#ifndef CORTEX_DIR_HPP
#define CORTEX_DIR_HPP

#include <memory>
#include "fs/dir_state.hpp"

using DirRef = std::unique_ptr<class Dir>;

class Dir {
public:
	virtual ~Dir() = default;

	virtual DirState GetDirState() = 0;

	virtual std::pair<bool, std::string> ReadEntireFile(std::string relative_path) = 0;

	virtual bool WriteEntireFile(std::string relative_path, const void *data, size_t size) = 0;

	bool WriteEntireFile(std::string relative_path, const std::string &content) {
		return WriteEntireFile(std::move(relative_path), content.data(), content.size());
	}

	static DirRef Create(std::string dir_path);
};

#endif//CORTEX_DIR_HPP