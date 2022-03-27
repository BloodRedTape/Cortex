#include "commit.hpp"
#include "error.hpp"

int main(int argc, char **argv) {
	std::string history_path;

	if (argc == 2) {
		history_path = argv[1];
	} else {
		Warning("No .history path supplied, using default one");
		history_path = "G:\\repo\\.history";
	}

	CommitHistory history;
	
	if(!history.LoadFrom(history_path))
		return Error("Can't load history from '%'", history_path);
	
	for (const Commit& commit : history) {
		Println("Prev: %,\nFileActionType: %,\nRelFilepath: %,\nUnixTime: %",
			commit.Previous,
			FileActionTypeString(commit.Action.Type),
			commit.Action.RelativeFilepath,
			commit.Action.ModificationTime.Seconds
		);
	}

	return 0;
}