add_executable(tests tests.cpp)
target_link_libraries(tests doubly_mapped_queue GTest::GTest Threads::Threads)

add_test(tests tests)
