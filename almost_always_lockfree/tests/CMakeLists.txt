add_executable(tests tests.cpp)
target_link_libraries(tests fmt::fmt GTest::GTest Threads::Threads)

add_test(tests tests)
