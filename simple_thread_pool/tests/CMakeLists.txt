file(GLOB LIB_SOURCES ${LIB_DIR}/src/*.cpp)

add_executable(tests tests.cpp ${LIB_SOURCES})

target_link_libraries(tests gtest gtest_main pthread)

add_test(tests tests)