set(TEST_SOURCES
        BoolBaseTests.cpp
        main.cpp
)

if (NOT TARGET bool_tests)
    add_executable(bool_tests ${TEST_SOURCES})
    #target_link_libraries(bool_tests bool gtest gtest_main)
    add_test(NAME bool_tests COMMAND bool_tests)
endif ()
