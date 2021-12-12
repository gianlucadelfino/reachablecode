file(GLOB LIB_SOURCES ${LIB_DIR}/src/*.cpp)

add_executable(main main.cpp ${LIB_SOURCES})
