#pragma once

#include "fs/dir_state.hpp"
#include "commit.hpp"

namespace UI {
void DockWindow();

void DirectoryWindow(const char *title, const DirState &state);

bool CommitHistoryWindow(const char *title, CommitHistory &history, size_t &selected);

}//namespace UI::
