#include <sstream>
#include "ui.hpp"
#include "imgui.h"
#include "imgui_internal.h"

namespace UI{

void DockWindow(ImGuiID id){

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y));
        ImGui::SetNextWindowPos({0, 0});
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_NoResize;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        flags |= ImGuiWindowFlags_NoMove;

        ImGui::Begin("__DOCK_WINDOW__", nullptr, flags);
        {
            ImGui::DockSpace(id);
        }
        ImGui::End();

        ImGui::PopStyleVar(2);
}

void DirectoryWindow(const char *title, const DirState& state){
    ImGui::Begin("Directory");
    
    if (ImGui::BeginTable("__Directory__", 1, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders))
    {
        for (int i = 0; i < state.size(); i++){
			const FileMeta &meta = state[i];

			ImGui::PushID(i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", meta.RelativeFilepath.c_str());
            //XXX: Show Time 
            //ImGui::TableNextColumn();
            //ImGui::Text(history[i].Action.RelativeFilepath.c_str(), &is, ImGuiSelectableFlags_SpanAllColumns);
			ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

bool CommitHistoryWindow(const char* title, CommitHistory& history, size_t &selected){
    bool changed = false;
	ImGui::Begin(title);
    ImGui::Text("Commits: %d", (int)history.Size());
    ImGui::Separator();
    ImGui::BeginChild("Child");
	if (ImGui::BeginTable("__HISTORY__", 1, ImGuiTableFlags_Resizable))
    {
        for (int i = 0; i < history.Size(); i++){
			const Commit &commit = history[i];

			bool is = selected == i;
            bool deleted = commit.Action.Type == FileActionType::Delete;
            ImVec4 icon_color = !deleted ? ImVec4(0.4, 0.9, 0.4, 1.0) : ImVec4(0.9, 0.1, 0.1, 1.0);
            const char *icon_text = deleted ? "D" : "W";
            std::string hash = (std::stringstream() << commit.Previous).str().substr(0, 6);

			ImGui::PushID(i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Button, icon_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, icon_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, icon_color);
            ImGui::Button(icon_text);
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            ImGui::Text("%s", hash.c_str());
            ImGui::SameLine();
            if(ImGui::Selectable(history[i].Action.RelativeFilepath.c_str(), &is, ImGuiSelectableFlags_SpanAllColumns)){
                selected = i;
                changed = true;
            }
			ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

	ImGui::End();
    return changed;
}

}//namespace UI::