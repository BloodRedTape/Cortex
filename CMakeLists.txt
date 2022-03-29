cmake_minimum_required(VERSION 3.8)

project(Cortex)

add_library(CortexCore STATIC
	"./core/commit.cpp"
	"./core/client_repository.cpp"
	"./core/win32_fs.cpp"
	"./core/fs/dir_state.cpp"
)

target_include_directories(CortexCore 
	PUBLIC "./core"
)

add_executable(CortexServer
	"./server/cortex.cpp"
)

target_link_libraries(CortexServer CortexCore)

add_executable(CortexHistoryDump 
	"./history_dump/history_dump.cpp"
)

target_link_libraries(CortexHistoryDump CortexCore)
