#pragma once

#include "commit.hpp"

class HistoryBrowser {
	static constexpr size_t InvalidSelection = -1;
private:
	CommitHistory m_History;
	size_t m_SelectedCommit = InvalidSelection;

	DirState m_State;

public:
	HistoryBrowser(const char *path);

	void Draw();
};