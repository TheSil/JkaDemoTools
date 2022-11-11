add_executable(DemoChatExtractor 
    chat_extractor.cc
 )

include_directories(${CMAKE_SOURCE_DIR}/DemoManipulator)

target_link_libraries(DemoChatExtractor DemoManipulator)
target_link_libraries(DemoChatExtractor ${CONAN_LIBS})

