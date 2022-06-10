include(Platform)



if(NOT CORTEX_PLATFORM_WINDOWS AND NOT CORTEX_PLATFORM_LINUX)
	message("History browser is not supported on this platform")
    set(CORTEX_HISTORY_BROWSER_SUPPORTED OFF)
else()
    set(CORTEX_HISTORY_BROWSER_SUPPORTED ON)
endif()

if(CORTEX_HISTORY_BROWSER_SUPPORTED)

    add_subdirectory(glfw)
    add_subdirectory(glad)

    set(CORTEX_HISTORY_BROWSER_SOURCES
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


    if(CORTEX_PLATFORM_WINDOWS)
        add_executable(CortexHistoryBrowser WIN32 ${CORTEX_HISTORY_BROWSER_SOURCES})
    endif()

    if(CORTEX_PLATFORM_LINUX)
        add_executable(CortexHistoryBrowser ${CORTEX_HISTORY_BROWSER_SOURCES})
    endif()

    target_link_libraries(CortexHistoryBrowser
        CortexCore
        glfw
        glad
    )

    target_include_directories(CortexHistoryBrowser
        PRIVATE "./imgui"
        PRIVATE "./imgui/backends"
    )

endif()