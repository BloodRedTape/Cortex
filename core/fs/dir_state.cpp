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

bool DirState::Has(const std::string& relative_filepath) {
	return Find(relative_filepath);
}
const FileState* DirState::Find(const std::string& relative_filepath)const {
	for(const FileState &state: *this){
		if(state.RelativeFilepath == relative_filepath)
			return &state;
	}
	return nullptr;
}
FileState* DirState::Find(const std::string& relative_filepath) {
	return const_cast<FileState *>(const_cast<const DirState*>(this)->Find(relative_filepath));
}

const FileState* DirState::Find(const FileState& other)const{
	return Find(other.RelativeFilepath);
}

void DirState::Remove(FileState *state){
	assert(state >= data() && state < data() + size());
	std::swap(back(), *state);
	pop_back();
}
