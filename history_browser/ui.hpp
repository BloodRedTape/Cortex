#pragma once

#include "fs/dir_state.hpp"
#include "commit.hpp"
#include "imgui.h"

namespace UI {
void DockWindow(ImGuiID id);

void DirectoryWindow(const char *title, const DirState &state);

bool CommitHistoryWindow(const char *title, CommitHistory &history, size_t &selected);

}//namespace UI::
