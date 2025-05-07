set(EXPR_HEADERS
        node.h
        op.h
        builder.h
)

set(EXPR_SOURCES
        builder.cpp
        node.cpp
)

add_library(expr STATIC ${EXPR_HEADERS} ${EXPR_HEADERS})
set_target_properties(expr PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS expr
        RUNTIME DESTINATION lib/canopy)

