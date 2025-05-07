#ifndef CANOPY_UTILS_PROFILE_H
#define CANOPY_UTILS_PROFILE_H

/**
* @file profile.h
* @author Arjun Earthperson
* @date 10/20/2024
* @brief This file contains the declaration of stats created from profiling runs.
*/

#include <cmath>
#include <iomanip>
#include <iostream>
#include <ostream>

/**
 * @namespace canopy::utils
 * @brief A namespace for canopy utility functions
 */
namespace canopy::utils {

    /**
     * @struct Profile
     * @brief A structure to hold the summary statistics created from profiling runs.
     * @tparam T The numeric type used to store this information
     */
    template <typename T>
    struct Profile {
        T min = std::numeric_limits<T>::quiet_NaN(); ///< The minimum value.
        T max = std::numeric_limits<T>::quiet_NaN(); ///< The maximum value.
        T sum = std::numeric_limits<T>::quiet_NaN(); ///< The sum of all values.
        T mean = std::numeric_limits<T>::quiet_NaN(); ///< The mean value.
        T variance = std::numeric_limits<T>::quiet_NaN(); ///< The variance.
        T stddev = std::numeric_limits<T>::quiet_NaN(); ///< The standard deviation.
        T p5th = std::numeric_limits<T>::quiet_NaN(); ///< The 5th percentile.
        T p95th = std::numeric_limits<T>::quiet_NaN(); ///< The 95th percentile.
        size_t runs{};

        /**
         * @brief Overloaded stream insertion operator to print the summary statistics.
         * @param os The output stream.
         * @param summary The summary statistics to print.
         * @return The output stream.
         */
        friend std::ostream &operator<<(std::ostream &os, const Profile<T> &summary) {
            auto precision = std::cout.precision() ;
            const auto width = static_cast<int>(2 + static_cast<std::streamsize>(10));
            os << std::setprecision(8) << std::setw(width) << std::setfill(' ') << std::scientific;
            os << "[min,  max] : " << "[" << summary.min << ", " << summary.max << "]" << std::endl;
            os << "[avg,  std] : " << "[" << summary.mean << ", " << summary.stddev << "]" << std::endl;
            os << "[5th, 95th] : " << "[" << summary.p5th << ", " << summary.p95th << "]" << std::endl;
            os << std::setprecision(static_cast<int>(precision));
            return os;
        }
    };
}
#endif // CANOPY_UTILS_PROFILE_H
