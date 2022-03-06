#include "filesystem.hpp"
#include <algorithm>

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

bool Match(const FileState& l, const FileState& r) {
	return l.RelativeFilepath == r.RelativeFilepath;
}

std::vector<FileAction> operator-(const DirState& nnew, const DirState& old) {
	std::vector<FileAction> result;

	for (const FileState& old_file : old) {
		auto it = std::find(nnew.begin(), nnew.end(), [old_file](const FileState &state){
			return state.RelativeFilepath == old_file.RelativeFilepath;
		});

		const bool found = it != nnew.end();

		if (!found) {
			result.emplace_back(FileActionType::Delete, UnixTime(), old_file.RelativeFilepath);
		}

		if (found) {
			if (old_file.ModificationTime.Seconds != it->ModificationTime.Seconds) {
				result.emplace_back(FileActionType::Update, it->ModificationTime, it->RelativeFilepath);
			}
		}
	}

	for (const FileState& new_file : nnew) {
		auto it = std::find(old.begin(), old.end(), [new_file](const FileState &state){
			return state.RelativeFilepath == new_file.RelativeFilepath;
		});

		const bool found = it != old.end();

		if (!found) {
			result.emplace_back(FileActionType::Create, new_file.ModificationTime, new_file.RelativeFilepath);
		}
	}

	return result;
}
