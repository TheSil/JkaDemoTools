cmake_minimum_required(VERSION 3.4)
project(JkaDemoTools)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(DemoCutter)
add_subdirectory(DemoSmoother)
add_subdirectory(DemoChatExtractor)
add_subdirectory(DemoManipulator)




