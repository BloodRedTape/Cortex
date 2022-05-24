#ifndef CORTEX_FILE_ACTION_HPP
#define CORTEX_FILE_ACTION_HPP

#include <cassert>
#include <vector>
#include "serializer.hpp"
#include "time.hpp"

enum class FileActionType: u32{
	Write  = 1,
	Delete = 2,
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
	case FileActionType::Write:  return "Write";
	case FileActionType::Delete: return "Delete";
	}
	assert(false);
	return "Shit happens";
}

struct FileAction{ 
	FileActionType Type;
	std::string RelativeFilepath;

	FileAction(FileActionType type, std::string relative_filepath):
		Type(type),
		RelativeFilepath(std::move(relative_filepath))
	{}
};

template<>
struct Serializer<FileAction> {
	static void Serialize(std::ostream& stream, const FileAction& value) {
		Serializer<FileActionType>::Serialize(stream, value.Type);
		Serializer<std::string>::Serialize(stream, value.RelativeFilepath);
	}
	static FileAction Deserialize(std::istream& stream) {
		FileActionType type  = Serializer<FileActionType>::Deserialize(stream);
		std::string rel_path = Serializer<std::string>::Deserialize(stream);

		return {
			type, 
			std::move(rel_path)
		};
	}
};

class FileActionAccumulator: private std::vector<FileAction>{
	using Super = std::vector<FileAction>;
private:
	friend class Serializer<FileActionAccumulator>;
public:
	void Add(FileAction action);
	
	using Super::begin;

	using Super::end;
};

inline void FileActionAccumulator::Add(FileAction new_action){
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
