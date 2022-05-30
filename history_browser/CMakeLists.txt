
add_subdirectory(glfw)
add_subdirectory(glad)

add_executable(CortexHistoryBrowser
	"main.cpp"
	"history_browser.cpp"
	"./imgui/imgui.cpp"
	"./imgui/imgui_widgets.cpp"
	"./imgui/imgui_tables.cpp"
	"./imgui/imgui_draw.cpp"
	"./imgui/imgui_demo.cpp"
	"./imgui/backends/imgui_impl_glfw.cpp"
	"./imgui/backends/imgui_impl_opengl3.cpp"
 "ui.hpp" "ui.cpp")

target_link_libraries(CortexHistoryBrowser
	CortexCore
	glfw
	glad
)

target_include_directories(CortexHistoryBrowser
	PRIVATE "./imgui"
	PRIVATE "./imgui/backends"
)