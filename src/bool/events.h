#ifndef CANOPY_EVENTS_H
#define CANOPY_EVENTS_H

#include <cstddef>

/**
 * @file events.h
 * @brief Events logic definitions
 *
 * Defines the templated input events structs/classes
 *
 * @author Arjun Earthperson
 * @date 10/30/2024
 */

template <typename size_type = std::size_t>
struct array {
    size_type x;
    //T *ary;
};

#endif //CANOPY_EVENTS_H
