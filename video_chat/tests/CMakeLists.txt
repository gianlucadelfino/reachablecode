add_executable(tests tests.cpp)
target_link_libraries(tests video_chat_utils GTest::GTest Threads::Threads)

add_test(tests tests)
