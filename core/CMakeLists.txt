include(Platform)

set(CORTEX_CORE_SOURCES
	"./commit.cpp"
	"./fs/dir_state.cpp"
	"./net/ip.cpp"
	"./net/udp_socket.cpp"
	"./net/tcp_socket.cpp"
	"./net/tcp_listener.cpp"
)

if(CORTEX_PLATFORM_WINDOWS)
    list(APPEND CORTEX_CORE_SOURCES
    	"./win32_fs.cpp"
	    "./win32_time.cpp"
	    "./win32_net.cpp"
    )
endif()

if(CORTEX_PLATFORM_LINUX)
    list(APPEND CORTEX_CORE_SOURCES
    	"./linux_fs.cpp"
	    "./linux_time.cpp"
	    "./linux_net.cpp"
    )
endif()

add_library(CortexCore STATIC ${CORTEX_CORE_SOURCES})

target_compile_features(CortexCore PUBLIC cxx_std_17)

if(CORTEX_PLATFORM_WINDOWS)
	target_link_libraries(CortexCore ws2_32)
endif()

if(CORTEX_PLATFORM_LINUX)
	target_link_libraries(CortexCore pthread)
endif()

target_include_directories(CortexCore 
	PUBLIC "./"
)