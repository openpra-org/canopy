/// @file
/// Convenience iterator adaptor to wrap find calls and results.

#pragma once

#include <utility>

namespace mef::openpsa {

/// Iterator adaptor for indication of container ``find`` call results.
/// Conveniently wraps common calls after ``find`` into implicit Boolean value.
///
/// @tparam Iterator  Iterator type belonging to the container.
template <class Iterator>
class find_iterator : public Iterator {
 public:
   /// Initializes the iterator as the result of ``find()``.
   ///
   /// @param[in] it  The result of ``find`` call.
   /// @param[in] it_end  The sentinel iterator indicator ``not-found``.
   find_iterator(Iterator&& it, const Iterator& it_end)
       : Iterator(std::move(it)), found_(*this != it_end) {}

   /// @returns true if the iterator indicates that the item is found.
   explicit operator bool() { return found_; }

 private:
   bool found_;  ///< Indicator of the lookup result.
};

/// Wraps ``container::find()`` calls for convenient and efficient lookup
/// with ``find_iterator`` adaptor.
///
/// @tparam T  Container type supporting ``find()`` and ``end()`` calls.
/// @tparam Arg  The argument type to the ``find()`` call.
///
/// @param[in] container  The container to operate upon.
/// @param[in] arg  The argument to the ``find()`` function.
///
/// @returns find_iterator wrapping the resultant iterator.
template <class T, typename Arg>
auto find(T&& container, Arg&& arg) {
   auto it = container.find(std::forward<Arg>(arg));
   return find_iterator<decltype(it)>(std::move(it), container.end());
}

}  // namespace ext
