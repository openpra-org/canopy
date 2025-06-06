#ifndef BOOLEXPR_UTIL_H
#define BOOLEXPR_UTIL_H

#include <concepts>
#include <type_traits>

namespace canopy::boolexpr::util {

template <class T>
concept Hashable = requires (T val) {
    { std::hash<T>{}(val) } -> std::convertible_to<std::size_t>;
};


template <typename T>
concept UnsignedInteger = std::integral<T> && std::is_unsigned_v<T>;

}

#endif
