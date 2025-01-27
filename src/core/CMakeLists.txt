set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CANOPY_SRC
        node.h
        layer.h
        working_set.h
        kernel/basic_event.h
        kernel/gate.h
        kernel/tally.h
        queue/layer_manager.h
        queue/kernel_builder.h
        queue/queueable.h
        analysis.cc
)

add_library(canopy SHARED ${CANOPY_SRC})

add_sycl_to_target(TARGET canopy SOURCES ${CANOPY_SRC})
set_target_properties(canopy PROPERTIES LINKER_LANGUAGE CXX)

#add_executable(canopy-cli main.cpp)
#add_sycl_to_target(TARGET canopy-cli canopy SOURCES main.cpp ${CANOPY_SRC})
