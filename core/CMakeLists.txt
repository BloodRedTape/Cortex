cmake_minimum_required(VERSION 3.8)

project(CortexCore)

add_library(CortexCore STATIC
	"commit.cpp"
	"client_repository.cpp"
	"filesystem.cpp"
	"win32_filesystem.cpp"
)

target_include_directories(CortexCore 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
