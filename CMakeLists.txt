cmake_minimum_required(VERSION 3.8)

project(Cortex)

add_library(CortexCore STATIC
	"./core/commit.cpp"
	
	"./core/win32_fs.cpp"
	"./core/win32_time.cpp"
	"./core/win32_net.cpp"
	"./core/fs/dir_state.cpp"
	"./core/net/ip.cpp"
	"./core/net/udp_socket.cpp"
	"./core/net/tcp_socket.cpp"
	"./core/net/tcp_listener.cpp"
)

target_compile_features(CortexCore PUBLIC cxx_std_17)

target_link_libraries(CortexCore ws2_32)

target_include_directories(CortexCore 
	PUBLIC "./core"
)

add_executable(CortexServer
	"./server/cortex.cpp"
)

target_link_libraries(CortexServer CortexCore)

add_executable(CortexClient
	"./client/cortex.cpp"
)

target_link_libraries(CortexClient CortexCore)

add_executable(CortexHistoryDump 
	"./history_dump/history_dump.cpp"
)

target_link_libraries(CortexHistoryDump CortexCore)
