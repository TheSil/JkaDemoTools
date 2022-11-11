add_executable(DemoSmoother 
    smoother.cc
 )

include_directories(${CMAKE_SOURCE_DIR}/DemoManipulator)

target_link_libraries(DemoSmoother DemoManipulator)
target_link_libraries(DemoSmoother ${CONAN_LIBS})

