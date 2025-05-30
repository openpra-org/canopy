#ifndef BOOLEXPR_UTIL_H
#define BOOLEXPR_UTIL_H

#include <concepts>

template <class T>
concept Hashable = requires (T val) {
    { std::hash<T>{}(val) } -> std::convertible_to<std::size_t>;
};

#endif
