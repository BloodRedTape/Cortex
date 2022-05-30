#include <fstream>
#include "imgui.h"
#include "imgui_internal.h"
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
{

}

void HistoryBrowser::Draw() {

	ImGuiID dockspaceID = ImGui::GetID("__DockSpace__");

	if(!ImGui::DockBuilderGetNode(dockspaceID)){
		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);
		ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetMainViewport()->Size);

		ImGuiID dock_main_id = dockspaceID;
		ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.7f, nullptr, &dock_main_id);
		ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.3f, nullptr, &dock_main_id);

		ImGui::DockBuilderDockWindow("History", dock_left_id);
		ImGui::DockBuilderDockWindow("Directory", dock_right_id);

		ImGui::DockBuilderFinish(dock_main_id);
	}

	UI::DockWindow(dockspaceID);
	UI::DirectoryWindow("Directory", m_State);
	if(UI::CommitHistoryWindow("History", m_History, m_SelectedCommit))
		m_State = m_History.TraceDirStateUntil(Hash(m_History[m_SelectedCommit]));
}


