set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SCRAM_CLI_UTILS_HEADERS
        ConstructOptions.h
        ConstructSettings.h
        XmlLogger.h
        ParseArguments.h
        RunScram.h
)

if (NOT TARGET scram-cli-utils)
    # Create utils
    add_library(scram-cli-utils STATIC ${SCRAM_CLI_UTILS_HEADERS})
    target_include_directories(scram-cli-utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(scram-cli-utils PROPERTIES LINKER_LANGUAGE CXX)
endif ()

# Link utils against Boost libraries
target_link_libraries(scram-cli-utils scram canopy ${LIBS})
