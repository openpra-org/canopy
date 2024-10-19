/*
    MIT License

    Copyright (c) 2024 OpenPRA Initiative

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
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
