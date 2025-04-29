add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE fmt::fmt gtest gtest_main Threads::Threads)

add_test(tests tests)
