/// @file
/// Helpful algorithm facilities as an extension to the STL or Boost.

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>

#include <boost/range/algorithm.hpp>

namespace mef::openpsa {

/// Determines if two sorted ranges intersect.
/// This function is complementary to std::set_intersection
/// when the actual intersection container is not needed.
///
/// @tparam Iterator1  Forward iterator type of the first range.
/// @tparam Iterator2  Forward iterator type of the second range.
///
/// @param[in] first1  Start of the first range.
/// @param[in] last1  End of the first range.
/// @param[in] first2  Start of the second range.
/// @param[in] last2  End of the second range.
///
/// @returns true if the [first1, last1) and [first2, last2) ranges intersect.
template <typename Iterator1, typename Iterator2>
bool intersects(Iterator1 first1, Iterator1 last1, Iterator2 first2,
               Iterator2 last2) noexcept {
   while (first1 != last1 && first2 != last2) {
       if (*first1 < *first2) {
           ++first1;
       } else if (*first2 < *first1) {
           ++first2;
       } else {
           return true;
       }
   }
   return false;
}

/// Range-based version of ``intersects``.
template <class SinglePassRange1, class SinglePassRange2>
bool intersects(const SinglePassRange1& rng1, const SinglePassRange2& rng2) {
   return intersects(boost::begin(rng1), boost::end(rng1), boost::begin(rng2),
                     boost::end(rng2));
}

/// Range-based versions of std algorithms missing in Boost.
/// @{
template <class SinglePassRange, class UnaryPredicate>
bool none_of(const SinglePassRange& rng, UnaryPredicate pred) {
   return boost::end(rng) == boost::find_if(rng, pred);
}
template <class SinglePassRange, class UnaryPredicate>
bool any_of(const SinglePassRange& rng, UnaryPredicate pred) {
   return !none_of(rng, pred);
}
template <class SinglePassRange, class UnaryPredicate>
bool all_of(const SinglePassRange& rng, UnaryPredicate pred) {
   return boost::end(rng) ==
          std::find_if_not(boost::begin(rng), boost::end(rng), pred);
}

/// Compares two floating point number within tolerance.
///
/// @param[in] lhs  The reference comparison value.
/// @param[in] rhs  The test value.
/// @param[in] tolerance  The percent tolerance for the difference.
///
/// @returns true if the values are close within the tolerance.
///
/// @note The implementation does not deal with overflows and underflows.
///       Therefore, the compared numbers must not be too large or small.
inline bool is_close(double lhs, double rhs, double tolerance) noexcept {
    assert(tolerance < 1 && "Invalid tolerance for float comparison.");
    return std::abs(lhs - rhs) <= std::abs(lhs) * tolerance;
}
/// @}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace detail {

// Rotates two discontinuous ranges to put *first2 where *first1 is.
//     If last1 == first2,
//     this would be equivalent to rotate(first1, first2, last2),
//     but instead the rotate "jumps" over the discontinuity [last1, first2) -
//     which need not be a valid range.
//     In order to make it faster,
//     the length of [first1, last1) is passed in as d1,
//     and d2 must be the length of [first2, last2).
//  In a perfect world, the d1 > d2 case would have used
//     swap_ranges and reverse_iterator,
//     but reverse_iterator is too inefficient.
template <class BidirIter>
void rotate_discontinuous(
    BidirIter first1, BidirIter last1,
    typename std::iterator_traits<BidirIter>::difference_type d1,
    BidirIter first2, BidirIter last2,
    typename std::iterator_traits<BidirIter>::difference_type d2) {
    using std::swap;
    if (d1 <= d2) {
        std::rotate(first2, std::swap_ranges(first1, last1, first2), last2);
    } else {
        BidirIter i1 = last1;
        while (first2 != last2)
            swap(*--i1, *--last2);
        std::rotate(first1, i1, last1);
    }
}

// Call f() for each combination of the elements
//    [first1, last1) + [first2, last2)
//    swapped/rotated into the range [first1, last1).
//    As long as f() returns false,
//    continue for every combination,
//    and then return [first1, last1) and [first2, last2)
//    to their original state.
//    If f() returns true, return immediately.
//  Does the absolute minimum amount of swapping to accomplish its task.
//  If f() always returns false, it will be called (d1+d2)!/(d1!*d2!) times.
template <class BidirIter, class Function>
bool combine_discontinuous(
    BidirIter first1, BidirIter last1,
    typename std::iterator_traits<BidirIter>::difference_type d1,
    BidirIter first2, BidirIter last2,
    typename std::iterator_traits<BidirIter>::difference_type d2,
    Function& f,  // NOLINT
    typename std::iterator_traits<BidirIter>::difference_type d = 0) {
    typedef typename std::iterator_traits<BidirIter>::difference_type D;
    using std::swap;
    if (d1 == 0 || d2 == 0)
        return f();
    if (d1 == 1) {
        for (BidirIter i2 = first2; i2 != last2; ++i2) {
            if (f())
                return true;
            swap(*first1, *i2);
        }
    } else {
        BidirIter f1p = std::next(first1);
        BidirIter i2 = first2;
        for (D d22 = d2; i2 != last2; ++i2, --d22) {
            if (combine_discontinuous(f1p, last1, d1 - 1, i2, last2, d22, f, d + 1))
                return true;
            swap(*first1, *i2);
        }
    }
    if (f())
        return true;
    if (d != 0) {
        rotate_discontinuous(first1, last1, d1, std::next(first2), last2, d2 - 1);
    } else {
        rotate_discontinuous(first1, last1, d1, first2, last2, d2);
    }
    return false;
}

// Creates a functor with no arguments which calls f_(first_, last_).
//   Also has a variant that takes two It and ignores them.
template <class Function, class It>
class bound_range {
    Function f_;
    It first_;
    It last_;

  public:
    bound_range(Function f, It first, It last)
        : f_(f), first_(first), last_(last) {}

    bool operator()() { return f_(first_, last_); }

    bool operator()(It, It) { return f_(first_, last_); }
};

}  // namespace detail

#endif  // DOXYGEN_SHOULD_SKIP_THIS

/// Repeatedly permutes the range [first, last) such that
/// the range [first, mid) represents each combination of the values
/// in [first, last) taken distance(first, mid) at a time.
/// For each permutation calls f(first, mid).
/// On each call, the range [mid, last) holds the values not in the
/// current permutation. If f returns true then returns immediately without
/// permuting the sequence any further.
/// Otherwise, after the last call to f, and prior to returning,
/// the range [first, last) is restored to its original order.
///
/// @param[in] first  The start of the range.
/// @param[in] mid  The end of the combination.
/// @param[in] last  The sentinel end of the range.
/// @param[in] f  The function to be called per combination.
///
/// @returns f.
///
/// @pre The type of *first shall satisfy the requirements of Swappable,
///      MoveConstructible and the requirements of MoveAssignable.
///      [first, mid) and [mid, last) are valid ranges.
///      Function shall meet the requirements of
///      MoveConstructible and MoveAssignable.
///      f is callable as f(first, mid)
///      and returns a type contextually convertible to bool.
///
/// @note If f always returns false,
///       it is called count_each_combination(first, mid, last) times.
///
/// @note The type referenced by *first
///       need not be EqualityComparable nor LessThanComparable.
///       The input range need not be sorted.
///       The algorithm does not take the values in the range [first, last)
///       into account in any way.
template <class BidirIter, class Function>
Function for_each_combination(BidirIter first, BidirIter mid, BidirIter last,
                              Function f) {
    detail::bound_range<Function&, BidirIter> wfunc(f, first, mid);
    detail::combine_discontinuous(first, mid, std::distance(first, mid), mid,
                                  last, std::distance(mid, last), wfunc);
    return f;
}

/// Helper function to calculate reciprocal of
/// nCk (n-choose-k) combination.
///
/// @param[in] n  The total number elements.
/// @param[in] k  Subset size.
///
/// @returns 1 / nCk
double CalculateCombinationReciprocal(int n, int k) {
    assert(n >= 0);
    assert(k >= 0);
    assert(n >= k);
    if (n - k > k)
        k = n - k;
    double result = 1;
    for (int i = 1; i <= n - k; ++i) {
        result *= static_cast<double>(i) / static_cast<double>(k + i);
    }
    return result;
}

}  // namespace ext
