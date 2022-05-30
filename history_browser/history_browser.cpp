#include <fstream>
#include "imgui.h"
#include "ui.hpp"
#include "history_browser.hpp"

std::string ReadEntireFile(const char* path) {
	std::fstream file(path);
	
	if(!file.is_open())
		return {};

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string content(size, 0);
	file.read(content.data(), size);
	return content;
}

HistoryBrowser::HistoryBrowser(const char* path):
	m_History(ReadEntireFile(path))
{}

void HistoryBrowser::Draw() {
	UI::DockWindow();
	UI::DirectoryWindow("Directory", m_State);

	if(UI::CommitHistoryWindow("History", m_History, m_SelectedCommit))
		m_State = m_History.TraceDirStateUntil(Hash(m_History[m_SelectedCommit]));
}


