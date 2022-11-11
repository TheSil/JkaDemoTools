cmake_minimum_required(VERSION 3.4)

add_executable(DemoSmoother 
    smoother.cc
    ../DemoManipulator/demo.cc
    ../DemoManipulator/huffman.cc
    ../DemoManipulator/instruction.cc
    ../DemoManipulator/message.cc
    ../DemoManipulator/messagebuffer.cc
    ../DemoManipulator/state.cc
 )

target_link_libraries(DemoSmoother ${CONAN_LIBS})

