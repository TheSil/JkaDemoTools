cmake_minimum_required(VERSION 3.4)

add_executable(DemoChatExtractor 
    chat_extractor.cc
    ../DemoManipulator/demo.cc
    ../DemoManipulator/huffman.cc
    ../DemoManipulator/instruction.cc
    ../DemoManipulator/message.cc
    ../DemoManipulator/messagebuffer.cc
    ../DemoManipulator/state.cc
 )

target_link_libraries(DemoChatExtractor ${CONAN_LIBS})

