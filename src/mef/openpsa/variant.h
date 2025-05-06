/// @file
/// Extra helper functions for std::variant.

#pragma once

#include <variant>

namespace mef::openpsa::variant {

/// Retrieves the stored value in the variant with a common type.
///
/// @tparam T  The type compatible with any type stored in the variant.
/// @tparam Ts  The types of values stored in the variant.
///
/// @param[in] var  The variant with argument values.
///
/// @returns The stored value cast to the type T.
template <typename T, typename... Ts>
T as(const std::variant<Ts...>& var) {
   return std::visit([](auto& arg) { return static_cast<T>(arg); }, var);
}

/// Workaround gcc 8 bug with std::variant swap.
///
/// @param[in,out] lhs
/// @param[in,out] rhs
///
/// @todo Require patched GCC 8 instead of this workaround.
template <typename... Ts>
void swap(std::variant<Ts...>& lhs, std::variant<Ts...>& rhs) noexcept {
#if __GNUC__ == 8 && __GNUC_MINOR__ < 3
   auto tmp = std::move(rhs);
   rhs = std::move(lhs);
   lhs = std::move(tmp);
#else
   lhs.swap(rhs);
#endif
}

}  // namespace ext
