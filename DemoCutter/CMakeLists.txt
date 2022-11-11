cmake_minimum_required(VERSION 3.4)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /Zc:__cplusplus")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(DemoCutter WIN32 
    main.cc
    cutter.cc cutter.h
    gui.cc gui.h
    gui_ui.h
    ../DemoManipulator/demo.cc ../DemoManipulator/demo.h
    ../DemoManipulator/huffman.cc
    ../DemoManipulator/instruction.cc ../DemoManipulator/instruction.h
    ../DemoManipulator/message.cc ../DemoManipulator/message.h
    ../DemoManipulator/messagebuffer.cc ../DemoManipulator/messagebuffer.h
    ../DemoManipulator/state.cc ../DemoManipulator/state.h
    ../DemoManipulator/defs.h
    ../DemoManipulator/netfields.h
 )

target_link_libraries(DemoCutter ${CONAN_LIBS})

