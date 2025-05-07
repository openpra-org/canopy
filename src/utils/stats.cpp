/**
* @file stats.cpp
* @author Arjun Earthperson
* @date 10/20/2024
* @brief Implementation file for statistical utility functions for estimating the mean, variance, and associated
* confidence intervals.
*/

#include "utils/stats.h"

#include <algorithm>
#include <cmath>
#include <boost/math/distributions/normal.hpp>
#include <utility>
#include <stdexcept>

namespace canopy::utils {

// Template function implementations

template<typename float_type, typename size_type>
float_type mean(const size_type count, const size_type samples) {
    return (static_cast<float_type>(count) / static_cast<float_type>(samples));
}

template<typename float_type>
float_type absolute_error(const float_type estimated, const float_type known) {
    return std::abs(estimated - known);
}

template<typename float_type>
float_type relative_error(const float_type error, const float_type known) {
    const auto hundred_times_error = static_cast<float_type>(100) * error;
    return (hundred_times_error / known);
}

template<typename float_type, typename size_type>
float_type variance(const float_type mean, const size_type samples) {
    const auto mean_squared = mean * mean;
    return ((mean - mean_squared) / static_cast<float_type>(samples));
}

template<typename float_type, typename size_type>
float_type standard_error(const float_type mean, const size_type samples) {
    const auto one_minus_mean = static_cast<float_type>(1) - mean;
    const auto numerator = mean * one_minus_mean;
    const auto term = numerator / static_cast<float_type>(samples);
    const auto square_root = std::sqrt(term);
    return square_root;
}

template<typename float_type, typename size_type>
std::pair<float_type, float_type> confidence_interval(const float_type mean, const size_type num_samples, const float_type confidence_level) {
    if (num_samples == 0) {
        throw std::invalid_argument("Number of samples must be greater than 0.");
    }

    // Calculate the z-score for the given confidence level
    const boost::math::normal_distribution<float_type> dist(0, 1);
    const float_type z = boost::math::quantile(dist, (1 + confidence_level) / 2);

    // Calculate the standard error
    const float_type standard_error_val = canopy::utils::standard_error<float_type, size_type>(mean, num_samples);

    // Calculate the margin of error
    float_type margin_of_error = z * standard_error_val;

    // Calculate the confidence interval limits
    float_type lower_limit = mean - margin_of_error;
    float_type upper_limit = mean + margin_of_error;

    // Ensure limits are within [0, 1]
    lower_limit = std::max(lower_limit, static_cast<float_type>(0.0));
    upper_limit = std::min(upper_limit, static_cast<float_type>(1.0));

    return std::make_pair(lower_limit, upper_limit);
}

// Explicit template instantiations for common types
template double mean<double, std::size_t>(const std::size_t, const std::size_t);
template float mean<float, std::size_t>(const std::size_t, const std::size_t);

template double absolute_error<double>(const double, const double);
template float absolute_error<float>(const float, const float);

template double relative_error<double>(const double, const double);
template float relative_error<float>(const float, const float);

template double variance<double, std::size_t>(const double, const std::size_t);
template float variance<float, std::size_t>(const float, const std::size_t);

template double standard_error<double, std::size_t>(const double, const std::size_t);
template float standard_error<float, std::size_t>(const float, const std::size_t);

template std::pair<double, double> confidence_interval<double, std::size_t>(const double, const std::size_t, const double);
template std::pair<float, float> confidence_interval<float, std::size_t>(const float, const std::size_t, const float);

// SummaryStatistics class implementation

// Constructor
template<typename float_type, typename size_type>
SummaryStatistics<float_type, size_type>::SummaryStatistics(const size_type trueTallies, const size_type totalTallies, const float_type knownProbability)
    : true_tallies_(trueTallies),
      total_tallies_(totalTallies),
      known_probability_(knownProbability)
{
    computed_mean_ = canopy::utils::mean<float_type, size_type>(true_tallies_, total_tallies_);
    computed_variance_ = canopy::utils::variance<float_type, size_type>(computed_mean_, total_tallies_);
    computed_absolute_error_ = canopy::utils::absolute_error<float_type>(computed_mean_, known_probability_);
    computed_relative_error_ = canopy::utils::relative_error<float_type>(computed_absolute_error_, known_probability_);
    computed_standard_error_ = canopy::utils::standard_error<float_type, size_type>(computed_mean_, total_tallies_);
    computed_p95_ci_ = canopy::utils::confidence_interval<float_type, size_type>(computed_mean_, total_tallies_, 0.95);
    computed_p99_ci_ = canopy::utils::confidence_interval<float_type, size_type>(computed_mean_, total_tallies_, 0.99);
}

// Explicit template instantiations for common types
template class SummaryStatistics<double, std::size_t>;
template class SummaryStatistics<float, std::size_t>;

// operator<< is defined inline in the header as a friend, so no need to re-implement here.

} // namespace canopy::utils