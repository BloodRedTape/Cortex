cmake_minimum_required(VERSION 3.8)

project(Cortex)

add_subdirectory(core)

add_executable(Cortex 
	"cortex.cpp"
)

target_link_libraries(Cortex CortexCore)
