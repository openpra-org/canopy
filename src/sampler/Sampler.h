#ifndef CANOPY_SAMPLER_H
#define CANOPY_SAMPLER_H

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/quantile.hpp>
#include <boost/accumulators/statistics/extended_p_square.hpp>
#include <stdexcept> // For std::out_of_range

// Namespace alias for convenience
namespace acc = boost::accumulators;

// Define the accumulator type with required statistical features
using accumulator_type = acc::accumulator_set<
        double,
        acc::features<
                acc::tag::mean,
                acc::tag::variance,
                acc::tag::extended_p_square // Required for quantile calculations
        >
>;

// Helper Functions to Extract Statistics

/**
 * @brief Computes the mean of the accumulated values.
 *
 * @param acc_set The accumulator containing the data.
 * @return The mean value.
 * @throws std::runtime_error if the accumulator is empty.
 */
double get_mean(const accumulator_type& acc_set) {
    if (acc_set.count() == 0) {
        throw std::runtime_error("Accumulator is empty. Cannot compute mean.");
    }
    return acc::mean(acc_set);
}

/**
 * @brief Computes the variance of the accumulated values.
 *
 * @param acc The accumulator containing the data.
 * @return The variance.
 */
double get_variance(const accumulator_type& acc_set) {
    return acc::variance(acc_set);
}

/**
 * @brief Computes the skewness of the accumulated values.
 *
 * @param acc The accumulator containing the data.
 * @return The skewness.
 */
double get_skewness(const accumulator_type& acc_set) {
    return acc::skewness(acc_set);
}

/**
 * @brief Computes the kurtosis of the accumulated values.
 *
 * @param acc The accumulator containing the data.
 * @return The kurtosis.
 */
double get_kurtosis(const accumulator_type& acc_set) {
    return acc::kurtosis(acc_set);
}

/**
 * @brief Computes the median of the accumulated values.
 *
 * @param acc The accumulator containing the data.
 * @return The median value.
 */
double get_median(const accumulator_type& acc_set) {
    return acc::median(acc_set);
}

/**
 * @brief Computes the specified quantile of the accumulated values.
 *
 * @param acc The accumulator containing the data.
 * @param probability The quantile probability (e.g., 0.05 for 5th percentile).
 * @return The quantile value.
 */
double get_quantile(const accumulator_type& acc_set, double probability) {
    // Ensure the probability is within [0,1]
    if (probability < 0.0 || probability > 1.0) {
        throw std::out_of_range("Quantile probability must be between 0 and 1.");
    }
    return acc::quantile(acc_set, probability);
}

#endif //CANOPY_SAMPLER_H
