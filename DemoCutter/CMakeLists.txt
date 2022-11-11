cmake_minimum_required(VERSION 3.4)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /Zc:__cplusplus")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(DemoCutter WIN32 
    main.cc
    cutter.cc
    gui.cc
    ../DemoManipulator/demo.cc
    ../DemoManipulator/huffman.cc
    ../DemoManipulator/instruction.cc
    ../DemoManipulator/message.cc
    ../DemoManipulator/messagebuffer.cc
    ../DemoManipulator/state.cc
 )

target_link_libraries(DemoCutter ${CONAN_LIBS})

