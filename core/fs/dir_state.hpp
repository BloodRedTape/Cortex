#ifndef CORTEX_DIR_STATE_HPP
#define CORTEX_DIR_STATE_HPP

#include <vector>
#include "fs/file_action.hpp"

using DirStateDiff = std::vector<FileAction>;

struct DirState: std::vector<FileState> {
	DirState() = default;

	DirState(DirState &&) = default;

	DirState &operator=(DirState &&) = default;

	~DirState() = default;

	DirStateDiff GetDiffFrom(const DirState &old);

	bool Has(const std::string &relative_filepath);

	const FileState* Find(const std::string& relative_filepath)const;

	FileState* Find(const std::string& relative_filepath);

	const FileState* Find(const FileState& other)const;

	void Remove(FileState *state);

	friend std::ostream &operator<<(std::ostream &stream, const DirState &state);

	friend std::istream &operator>>(std::istream &stream, DirState &state);
};

#endif//CORTEX_DIR_STATE_HPP
