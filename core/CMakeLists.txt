cmake_minimum_required(VERSION 3.8)

project(CortexCore)

add_library(CortexCore STATIC
	"commit.cpp"
	"client_repository.cpp"
	
	"win32_fs.cpp"
	"fs/file_action.hpp"  "fs/dir_state.hpp" "fs/dir_state.cpp" "fs/dir_watcher.hpp" "fs/dir.hpp"  )

target_include_directories(CortexCore 
	PUBLIC "./"
)

