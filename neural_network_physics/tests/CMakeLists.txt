add_executable(tests tests.cpp)
target_link_libraries(tests GTest::GTest network Threads::Threads)

add_test(tests tests)
