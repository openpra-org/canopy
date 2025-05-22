find_package(LibXml2 REQUIRED)
find_package(_boost REQUIRED)

set(MEF_OPENPSA_HEADERS
        alignment.h
        error.h
        find_iterator.h
        model.h
        multi_index.h
        phase.h
        element.h
        linear_set.h
        instruction.h
        expression.h
        algorithm.h
        event.h
        variant.h
        ccf_group.h
        expr/numerical.h
        expr/constant.h
        event_tree.h
        expr/extern.h
        expr/test_event.h
        fault_tree.h
        parameter.h
        substitution.h
        initializer.h
        settings.h
        cycle.h
        expr/boolean.h
        expr/conditional.h
        expr/exponential.h
        expr/random_deviate.h
        env.h
)

set(MEF_OPENPSA_SOURCES
        event.cpp
        initializer.cpp
)

add_library(mef_openpsa STATIC ${MEF_OPENPSA_SOURCES} ${MEF_OPENPSA_HEADERS})
set_target_properties(mef_openpsa PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(mef_openpsa PRIVATE io_xml Boost::range Boost::multi_index Boost::exception Boost::icl Boost::math Boost::filesystem Boost::dll LibXml2::LibXml2)


install(TARGETS mef_openpsa
        RUNTIME DESTINATION lib/canopy)