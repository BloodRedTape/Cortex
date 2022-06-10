cmake_minimum_required(VERSION 3.8)

project(Cortex)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Platform)

add_definitions(
	-DCORTEX_PLATFORM_WINDOWS=${CORTEX_PLATFORM_WINDOWS}
	-DCORTEX_PLATFORM_LINUX=${CORTEX_PLATFORM_LINUX}
	-DCORTEX_PLATFORM_MACOS=${CORTEX_PLATFORM_MACOS}
	-DCORTEX_PLATFORM_IOS=${CORTEX_PLATFORM_IOS}
	-DCORTEX_PLATFORM_ANDROID=${CORTEX_PLATFORM_ANDROID}
)

add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(client_android)
add_subdirectory(server)
add_subdirectory(history_dump)
add_subdirectory(history_browser)

