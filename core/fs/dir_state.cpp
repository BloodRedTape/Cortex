#include "dir_state.hpp"
#include "fs/dir_state.hpp"
#include "utils.hpp"

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

	for (const FileMeta& old_file : old) {
		const FileMeta *it = this->Find(old_file);

		const bool found = it != nullptr;

		if (!found) { // We should check for deletion first to avoid strange order bugs
			result.emplace_back(FileActionType::Delete, old_file.RelativeFilepath);
			continue;
		}
		
		if (old_file.ModificationTime.Seconds != it->ModificationTime.Seconds) 
			result.emplace_back(FileActionType::Write, it->RelativeFilepath);
	}

	for (const FileMeta& new_file : *this) {
		const FileMeta *it = old.Find(new_file);

		const bool found = it != nullptr;

		if (!found)
			result.emplace_back(FileActionType::Write, new_file.RelativeFilepath);
	}

	return result;
}

bool DirState::Has(const std::string& relative_filepath) {
	return Find(relative_filepath);
}
const FileMeta* DirState::Find(const std::string& relative_filepath)const {
	for(const FileMeta &state: *this){
		if(state.RelativeFilepath == relative_filepath)
			return &state;
	}
	return nullptr;
}
FileMeta* DirState::Find(const std::string& relative_filepath) {
	return const_cast<FileMeta *>(const_cast<const DirState*>(this)->Find(relative_filepath));
}

const FileMeta* DirState::Find(const FileMeta& other)const{
	return Find(other.RelativeFilepath);
}

void DirState::Remove(FileMeta *state){
	assert(state >= data() && state < data() + size());
	std::swap(back(), *state);
	pop_back();
}
