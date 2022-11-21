file(GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(doubly_mapped_queue STATIC ${LIB_SOURCES})
