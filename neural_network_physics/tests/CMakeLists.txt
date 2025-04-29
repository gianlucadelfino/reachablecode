add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE gtest gtest_main network Threads::Threads)

add_test(tests tests)
