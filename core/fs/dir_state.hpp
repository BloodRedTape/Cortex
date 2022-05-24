#ifndef CORTEX_DIR_STATE_HPP
#define CORTEX_DIR_STATE_HPP

#include <vector>
#include "fs/file_action.hpp"

using DirStateDiff = std::vector<FileAction>;

struct FileMeta {
	std::string RelativeFilepath;
	UnixTime ModificationTime;

	FileMeta(std::string path, UnixTime time):
		RelativeFilepath(std::move(path)),
		ModificationTime(std::move(time))
	{}
};

struct DirState: std::vector<FileMeta> {
	DirState() = default;

	DirState(DirState &&) = default;

	DirState &operator=(DirState &&) = default;

	~DirState() = default;

	DirStateDiff GetDiffFrom(const DirState &old);

	bool Has(const std::string &relative_filepath);

	const FileMeta* Find(const std::string& relative_filepath)const;

	FileMeta* Find(const std::string& relative_filepath);

	const FileMeta* Find(const FileMeta& other)const;

	void Apply(const FileAction &action, UnixTime action_time);

	void Remove(FileMeta *state);

	friend std::ostream &operator<<(std::ostream &stream, const DirState &state);

	friend std::istream &operator>>(std::istream &stream, DirState &state);
};

#endif//CORTEX_DIR_STATE_HPP
