cmake_minimum_required(VERSION 3.10.2)

add_subdirectory("../../../../../" CortexNative)

add_library(CortexClientAndroid SHARED native-lib.cpp)
