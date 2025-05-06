set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXPR_HEADERS
        node.h
        op.h
        builder.cpp
        builder.h
)

set(EXPR_SOURCES
#        bool.cpp
)

#find_package(_boost REQUIRED COMPONENTS program_options serialization)

add_library(expr STATIC ${EXPR_HEADERS} ${EXPR_HEADERS})
set_target_properties(expr PROPERTIES LINKER_LANGUAGE CXX)
#target_link_libraries(utils boost_program_options boost_serialization)

install(TARGETS expr
        RUNTIME DESTINATION lib/canopy)

