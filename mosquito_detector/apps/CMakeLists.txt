file(GLOB LIB_SOURCES ${LIB_DIR}/src/*.cpp)

include_directories(SYSTEM ${COMMON_INCLUDE})

add_executable(main main.cpp ${LIB_SOURCES})
target_link_libraries(main ${OpenCV_LIBS})