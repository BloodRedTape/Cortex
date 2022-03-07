#include "filesystem.hpp"

std::ostream& operator<<(std::ostream& stream, const DirState& state) {
	for (const auto& file_state : state)
		stream << file_state.RelativeFilepath << ':' << file_state.ModificationTime.Seconds << std::endl;
	return stream;
}

std::istream& operator>>(std::istream& stream, DirState& state) {
	for (std::string file_state; std::getline(stream, file_state); ) {
		auto pos = file_state.find_first_of(':');
		std::string path = file_state.substr(0, pos);
		u64 seconds = std::atoll(&file_state[pos + 1]);
		state.emplace_back(std::move(path), UnixTime{seconds});
	}
	return stream;
}

DirStateDiff DirState::GetDiffFrom(const DirState& old) {
	DirStateDiff result;

	for (const FileState& old_file : old) {
		const FileState *it = this->Find(old_file);

		const bool found = it != nullptr;

		if (!found) { // We should check for deletion first to avoid strange order bugs
			result.emplace_back(FileActionType::Delete, UnixTime(), old_file.RelativeFilepath);
			continue;
		}
		
		if (old_file.ModificationTime.Seconds != it->ModificationTime.Seconds) 
			result.emplace_back(FileActionType::Update, it->ModificationTime, it->RelativeFilepath);
	}

	for (const FileState& new_file : *this) {
		const FileState *it = old.Find(new_file);

		const bool found = it != nullptr;

		if (!found)
			result.emplace_back(FileActionType::Create, new_file.ModificationTime, new_file.RelativeFilepath);
	}

	return result;
}

const FileState* DirState::Find(const FileState& other)const{
	for(const FileState &state: *this){
		if(state.RelativeFilepath == other.RelativeFilepath)
			return &state;
	}
	return nullptr;
}
