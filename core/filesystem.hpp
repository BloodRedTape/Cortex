#ifndef CORTEX_FILESYSTEM_HPP
#define CORTEX_FILESYSTEM_HPP

#include <memory>
#include <functional>
#include <string>
#include <cassert>
#include <ostream>
#include <istream>
#include <vector>
#include <regex>
#include "time.hpp"
#include "serializer.hpp"

enum class FileActionType {
	Create = 0,
	Delete = 1,
	Update = 2
};

template <>
struct Serializer<FileActionType>{
	static void Serialize(std::ostream& stream, const FileActionType& value) {
		Serializer<u32>::Serialize(stream, (u32)value);
	}
	static FileActionType Deserialize(std::istream& stream) {
		return (FileActionType)Serializer<u32>::Deserialize(stream);
	}
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

template<>
struct Serializer<FileAction> {
	static void Serialize(std::ostream& stream, const FileAction& value) {
		Serializer<std::string>::Serialize(stream, value.RelativeFilepath);
		Serializer<UnixTime>::Serialize(stream, value.ModificationTime);
		Serializer<FileActionType>::Serialize(stream, value.Type);
	}
	static FileAction Deserialize(std::istream& stream) {
		std::string rel_path = Serializer<std::string>::Deserialize(stream);
		UnixTime mod_time    = Serializer<UnixTime>::Deserialize(stream);
		FileActionType type  = Serializer<FileActionType>::Deserialize(stream);

		return {
			type, 
			mod_time,
			std::move(rel_path)
		};
	}
};

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

	friend std::ostream &operator<<(std::ostream &stream, const DirState &state);

	friend std::istream &operator>>(std::istream &stream, DirState &state);
};

using DirWatcherRef = std::unique_ptr<class DirWatcher>;

class IgnoreList {
private:
	std::vector<std::regex> m_Ignorers;
public:
	IgnoreList() = default;

	IgnoreList(std::initializer_list<std::regex> list) {
		m_Ignorers.reserve(list.size());
		for(const auto &regex: list)
			Add(regex);
	}

	IgnoreList(IgnoreList &&) = default;

	IgnoreList &operator=(IgnoreList &&) = default;

	void Add(std::regex regex) {
		m_Ignorers.push_back(std::move(regex));
	}

	bool ShouldBeIgnored(const std::string& item) {
		for (const std::regex& regex : m_Ignorers) {
			if(std::regex_match(item, regex))
				return true;
		}
		return false;
	}
};

class DirWatcher {
public:
	using OnDirChangedCallback = std::function<void(FileAction)>;
public:
	virtual ~DirWatcher() = default;

	virtual bool DispatchChanges() = 0;

	virtual DirState GetDirState() = 0;
	
	static DirWatcherRef Create(std::string dir_path, OnDirChangedCallback callback, IgnoreList ignore_list = {}, DirState initial_state = {}, bool is_blocking = true);
};

#endif //CORTEX_FILESYSTEM_HPP