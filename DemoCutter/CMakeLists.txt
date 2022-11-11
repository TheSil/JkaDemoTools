list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /Zc:__cplusplus")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(DemoCutter WIN32
    main.cc
    resource.rc
    cutter.cc cutter.h
    gui.cc gui.h
    gui_ui.h
)

include_directories(${CMAKE_SOURCE_DIR}/DemoManipulator)

target_link_libraries(DemoCutter DemoManipulator)
target_link_libraries(DemoCutter ${CONAN_LIBS})

