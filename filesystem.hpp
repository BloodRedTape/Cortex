#ifndef CORTEX_FILESYSTEM_HPP
#define CORTEX_FILESYSTEM_HPP

#include <memory>
#include <functional>
#include <string>
#include "time.hpp"

enum class FileActionType {
	Create = 0,
	Delete = 1,
	Update = 2
};

inline const char* FileActionTypeString(FileActionType type) {
	switch (type)
	{
	case FileActionType::Create: return "Create";
	case FileActionType::Delete: return "Delete";
	case FileActionType::Update: return "Update";
	}
}

struct FileAction {
	FileActionType Type;
	UnixTime    Time;
	std::string RelativeFilepath;

	FileAction(FileActionType type, UnixTime time, std::string relative_filepath):
		Type(type),
		Time(time),
		RelativeFilepath(std::move(relative_filepath))
	{}
};

#endif //CORTEX_FILESYSTEM_HPP