include(Platform)

add_library(CortexCore STATIC
	"./commit.cpp"
	
	"./win32_fs.cpp"
	"./win32_time.cpp"
	"./win32_net.cpp"
	"./fs/dir_state.cpp"
	"./net/ip.cpp"
	"./net/udp_socket.cpp"
	"./net/tcp_socket.cpp"
	"./net/tcp_listener.cpp"
)

target_compile_features(CortexCore PUBLIC cxx_std_17)

if(CORTEX_PLATFORM_WINDOWS)
	target_link_libraries(CortexCore ws2_32)
endif()

target_include_directories(CortexCore 
	PUBLIC "./"
)