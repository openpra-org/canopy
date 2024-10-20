#ifndef CANOPY_STATS_H
#define CANOPY_STATS_H

/**
* @file Stats.h
* @author Arjun Earthperson
* @date 10/18/2024
* @brief This file contains wrappers for statistical helper functions.
*/

#include <algorithm>
#include <cmath>
#include <numeric>
#include <ostream>
#include <vector>
#include <iostream>
#include <iomanip>

#include <boost/math/distributions/normal.hpp>
#include <utility>
#include <stdexcept>

/**
 * @namespace canopy::stats
 * @brief A namespace for statistical functions and structures.
 */
namespace canopy::stats {

    template<typename T>
    std::pair<T, T> confidence_interval(T mean, std::size_t num_samples, T confidence_level) {
        if (num_samples == 0) {
            throw std::invalid_argument("Number of samples must be greater than 0.");
        }

        // Calculate the z-score for the given confidence level
        boost::math::normal_distribution<T> dist(0, 1);
        T z = boost::math::quantile(dist, (1 + confidence_level) / 2);

        // Calculate the standard error
        T standard_error = std::sqrt(mean * (1.0 - mean) / num_samples);

        // Calculate the margin of error
        T margin_of_error = z * standard_error;

        // Calculate the confidence interval limits
        T lower_limit = mean - margin_of_error;
        T upper_limit = mean + margin_of_error;

        // Ensure limits are within [0, 1]
        lower_limit = std::max(lower_limit, static_cast<T>(0.0));
        upper_limit = std::min(upper_limit, static_cast<T>(1.0));

        return std::make_pair(lower_limit, upper_limit);
    }
}

#endif //CANOPY_STATS_H
