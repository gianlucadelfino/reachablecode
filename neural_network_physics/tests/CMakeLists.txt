add_executable(tests tests.cpp)
target_link_libraries(tests GTest::GTest gtest_main pthread)

add_test(tests tests)
