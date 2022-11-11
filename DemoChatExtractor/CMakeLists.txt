cmake_minimum_required(VERSION 3.4)

add_executable(DemoChatExtractor 
    chat_extractor.cc
    ../DemoManipulator/demo.cc ../DemoManipulator/demo.h
    ../DemoManipulator/huffman.cc
    ../DemoManipulator/instruction.cc ../DemoManipulator/instruction.h
    ../DemoManipulator/message.cc ../DemoManipulator/message.h
    ../DemoManipulator/messagebuffer.cc ../DemoManipulator/messagebuffer.h
    ../DemoManipulator/state.cc ../DemoManipulator/state.h
    ../DemoManipulator/defs.h
    ../DemoManipulator/netfields.h
 )

target_link_libraries(DemoChatExtractor ${CONAN_LIBS})

