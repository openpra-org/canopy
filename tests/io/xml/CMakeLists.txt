add_executable(test_io_xml openpsa.cpp)

target_link_libraries(test_io_xml PRIVATE io_xml Catch2::Catch2WithMain)