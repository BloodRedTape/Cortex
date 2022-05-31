include(Platform)

add_subdirectory(glfw)
add_subdirectory(glad)

if(NOT CORTEX_PLATFORM_WINDOWS)
	message(FATAL_ERROR "History browser is not supported on this platform")
endif()

add_executable(CortexHistoryBrowser WIN32
	"main.cpp"
	"ui.cpp"
	"history_browser.cpp"
	"./imgui/imgui.cpp"
	"./imgui/imgui_widgets.cpp"
	"./imgui/imgui_tables.cpp"
	"./imgui/imgui_draw.cpp"
	"./imgui/imgui_demo.cpp"
	"./imgui/backends/imgui_impl_glfw.cpp"
	"./imgui/backends/imgui_impl_opengl3.cpp"
)

target_link_libraries(CortexHistoryBrowser
	CortexCore
	glfw
	glad
)

target_include_directories(CortexHistoryBrowser
	PRIVATE "./imgui"
	PRIVATE "./imgui/backends"
)