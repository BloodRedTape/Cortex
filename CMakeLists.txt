cmake_minimum_required(VERSION 3.8)

project(Cortex)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Platform)

add_compile_definitions(
	CORTEX_PLATFORM_WINDOWS=${CORTEX_PLATFORM_WINDOWS}
	CORTEX_PLATFORM_LINUX=${CORTEX_PLATFORM_LINUX}
	CORTEX_PLATFORM_MACOS=${CORTEX_PLATFORM_MACOS}
	CORTEX_PLATFORM_IOS=${CORTEX_PLATFORM_IOS}
	CORTEX_PLATFORM_ANDROID=${CORTEX_PLATFORM_ANDROID}
)

add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(history_dump)
add_subdirectory(history_browser)

