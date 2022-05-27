#ifndef CORTEX_FILE_ACTION_HPP
#define CORTEX_FILE_ACTION_HPP

#include <vector>
#include "serializer.hpp"
#include "time.hpp"

struct FileData{
	std::string RelativeFilepath;
	std::string Content;
};

template <>
struct Serializer<FileData>{
	static void Serialize(std::ostream& stream, const FileData& value) {
		Serializer<std::string>::Serialize(stream, value.RelativeFilepath);
		Serializer<std::string>::Serialize(stream, value.Content);
	}
	static FileData Deserialize(std::istream& stream) {
		FileData file;
		file.RelativeFilepath = Serializer<std::string>::Deserialize(stream);
		file.Content = Serializer<std::string>::Deserialize(stream);
		return file;
	}
};

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
	CX_ASSERT(false);
	return "Shit happens";
}

struct FileAction{ 
	FileActionType	Type;
	UnixTime		Time;
	std::string		RelativeFilepath;

	FileAction(FileActionType type, UnixTime time, std::string relative_filepath):
		Type(type),
		Time(time),
		RelativeFilepath(std::move(relative_filepath))
	{}
};

template<>
struct Serializer<FileAction> {
	static void Serialize(std::ostream& stream, const FileAction& value) {
		Serializer<FileActionType>::Serialize(stream, value.Type);
		Serializer<UnixTime>::Serialize(stream, value.Time);
		Serializer<std::string>::Serialize(stream, value.RelativeFilepath);
	}
	static FileAction Deserialize(std::istream& stream) {
		FileActionType type  = Serializer<FileActionType>::Deserialize(stream);
		UnixTime time		 = Serializer<UnixTime>::Deserialize(stream);
		std::string rel_path = Serializer<std::string>::Deserialize(stream);

		return {
			type, 
			time,
			std::move(rel_path)
		};
	}
};

class FileActionAccumulator: private std::vector<FileAction>{
	using Super = std::vector<FileAction>;
private:
	friend class Serializer<FileActionAccumulator>;
public:
	FileActionAccumulator() = default;

	FileActionAccumulator(std::vector<FileAction> actions);

	void Add(FileAction action);
	
	using Super::begin;

	using Super::end;

	std::vector<FileAction> ToVector()const {
		return *this;
	}

	void Clear() {
		Super::clear();
	}

	size_t Size()const {
		return Super::size();
	}

	void OverrideIfConflicted(const FileAction &action);
};

template<>
struct Serializer<FileActionAccumulator> {
	static void Serialize(std::ostream& stream, const FileActionAccumulator& value) {
		Serializer<std::vector<FileAction>>::Serialize(stream, value);
	}
	static FileActionAccumulator Deserialize(std::istream& stream) {
		return {Serializer<std::vector<FileAction>>::Deserialize(stream)};
	}
};

inline FileActionAccumulator::FileActionAccumulator(std::vector<FileAction> actions){
	for (auto action : actions)
		Add(action);
}

inline void FileActionAccumulator::Add(FileAction new_action){
	bool is_added = false;
	for (auto &action : *this) {
		if (action.RelativeFilepath == new_action.RelativeFilepath) {
			if (new_action.Type == FileActionType::Delete) {
				std::swap(action, Super::back());
				Super::pop_back();
			} else if (new_action.Type == FileActionType::Write) {
				action.Time = new_action.Time;
			} else {
				CX_ASSERT(false);
			}
			is_added = true;
			break;
		}
	}
	if(!is_added)
		Super::push_back(std::move(new_action));
}

inline void FileActionAccumulator::OverrideIfConflicted(const FileAction &candidate){
	FileAction *conflicted = nullptr;

	for (FileAction &action : *this) {
		if (action.RelativeFilepath == candidate.RelativeFilepath) {
			conflicted = &action;
			break;
		}
	}

	if (conflicted) {
		std::swap(*conflicted, Super::back());
		Super::pop_back();
	}
}

#endif//CORTEX_FILE_ACTION_HPP
