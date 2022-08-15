add_executable(tests tests.cpp)
target_link_libraries(tests gtest gtest_main pthread)

add_test(tests tests)
