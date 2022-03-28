#include "commit.hpp"
#include "error.hpp"
#include "fs/dir.hpp"

int main(int argc, char **argv) {
	std::string history_path;

	if (argc == 2) {
		history_path = argv[1];
	} else {
		Warning("No .history path supplied, using default one");
		history_path = "W:\\Dev\\Cortex\\out\\repo\\";
	}


	DirRef dir = Dir::Create(history_path);
	auto res = dir->ReadEntireFile(".history");

	if(!res.first)
		return Error("Can't load history from '%'", history_path);

	CommitHistory history(res.second);
	
	for (const Commit& commit : history) {
		Println("Prev: %,\nFileActionType: %,\nRelFilepath: %,\nUnixTime: %\n",
			commit.Previous,
			FileActionTypeString(commit.Action.Type),
			commit.Action.RelativeFilepath,
			commit.Action.ModificationTime.Seconds
		);
	}

	return 0;
}