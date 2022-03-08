cmake_minimum_required(VERSION 3.8)

project(Cortex)

add_executable(Cortex 
	"cortex.cpp"
	"commit.cpp" 
	"repository.cpp"
	"filesystem.cpp"
	"win32_filesystem.cpp"
 "error.hpp")
