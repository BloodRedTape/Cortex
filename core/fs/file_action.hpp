#ifndef CORTEX_FILE_ACTION_HPP
#define CORTEX_FILE_ACTION_HPP

#include <cassert>
#include <vector>
#include "serializer.hpp"
#include "time.hpp"

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

class FileActionAccumulator: private std::vector<FileAction>{
	using Super = std::vector<FileAction>;
public:
	void Add(FileAction action);
	
	using Super::begin;

	using Super::end;
};

void FileActionAccumulator::Add(FileAction new_action){
	bool is_added = false;
	for (auto &action : *this) {
		if (action.RelativeFilepath == new_action.RelativeFilepath) {
			action.Type = new_action.Type;
			is_added = true;
			break;
		}
	}
	if(!is_added)
		Super::push_back(std::move(new_action));
}

#endif//CORTEX_FILE_ACTION_HPP
