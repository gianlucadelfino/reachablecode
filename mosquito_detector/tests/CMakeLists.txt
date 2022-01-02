file(GLOB LIB_SOURCES ${LIB_DIR}/src/*.cpp)

include_directories(SYSTEM ${COMMON_INCLUDE})
add_executable(tests tests.cpp ${LIB_SOURCES})

target_link_libraries(tests gtest gtest_main pthread ${OpenCV_LIBS})

add_test(tests tests)