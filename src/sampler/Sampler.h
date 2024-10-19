/*
    Copyright (C) 2024 OpenPRA Initiative

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CANOPY_SAMPLER_H
#define CANOPY_SAMPLER_H

#include <boost/accumulators/accumulators.hpp>

// Namespace alias for convenience
namespace acc = boost::accumulators;

// Define the accumulator type with required statistical features
typedef acc::accumulator_set<
        double,
        acc::features<
                acc::tag::mean,
                acc::tag::variance,
                acc::tag::skewness,
                acc::tag::kurtosis,
                acc::tag::median,
                acc::tag::quantile
        >
> accumulator_type;

// Helper Functions to Extract Statistics

/**
 * @brief Computes the mean of the accumulated values.
 *
 * @param acc The accumulator containing the data.
 * @return The mean value.
 */
double get_mean(const accumulator_type& acc_set) {
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
