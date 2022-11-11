cmake_minimum_required(VERSION 2.8.12)
project(JkaDemoTools)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup(TARGETS KEEP_RPATHS)
conan_basic_setup(KEEP_RPATHS)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_subdirectory(DemoCutter)




