add_executable(reader reader.cpp)

target_link_libraries(reader
        ${CMAKE_DL_LIBS}
        doubly_mapped_queue
    )

add_executable(writer writer.cpp)

target_link_libraries(writer
        ${CMAKE_DL_LIBS}
        doubly_mapped_queue
    )
