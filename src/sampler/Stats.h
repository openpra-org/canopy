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

#include "utils/json.h"

/**
 * @namespace Canopy::Sampler::Stats
 * @brief A namespace for statistical functions and structures.
 */
namespace Canopy::Sampler::Stats {

/**
 * @struct Summary
 * @brief A structure to hold the summary statistics of a vector or matrix.
 * @tparam T The type of the elements in the vector or matrix.
 */
    template<typename T>
    struct Summary {
        T min = std::numeric_limits<T>::quiet_NaN(); ///< The minimum value.
        T max = std::numeric_limits<T>::quiet_NaN(); ///< The maximum value.
        T sum = std::numeric_limits<T>::quiet_NaN(); ///< The sum of all values.
        T mean = std::numeric_limits<T>::quiet_NaN(); ///< The mean value.
        T variance = std::numeric_limits<T>::quiet_NaN(); ///< The variance.
        T stddev = std::numeric_limits<T>::quiet_NaN(); ///< The standard deviation.
        T median = std::numeric_limits<T>::quiet_NaN(); ///< The median value.
        T p5th = std::numeric_limits<T>::quiet_NaN(); ///< The 5th percentile.
        T p95th = std::numeric_limits<T>::quiet_NaN(); ///< The 95th percentile.
        size_t runs{};
        size_t maxBytes = std::numeric_limits<size_t>::quiet_NaN();

        /**
         * @brief Overloaded stream insertion operator to print the summary statistics.
         * @param os The output stream.
         * @param summary The summary statistics to print.
         * @return The output stream.
         */
        friend std::ostream &operator<<(std::ostream &os, const Summary &summary) {
            auto precision = std::cout.precision();
            const auto width = static_cast<int>(2 + static_cast<std::streamsize>(10));
            os << std::setprecision(2) << std::setw(width) << std::setfill(' ') << std::scientific;
            os << ":::::: SUM: " << summary.sum << " :::::::: VARIANCE: " << summary.variance << " :::::::: MEDIAN: "
               << summary.median << " :::::\n";
            os << ":::::: {     MIN,      MAX} : (AVERAGE  ± STD.DEV.) : [PCT_05th, PCT_95th] :::::\n";
            os << ":::::: {" << summary.min << ", " << summary.max << "} ";
            os << ": (" << summary.mean << " ± " << summary.stddev << ") : [";
            os << summary.p5th << ", " << summary.p95th << "] :::::";
            os << "\n:::::: Estimated maximum allocated memory [bytes]: " << summary.maxBytes
               << " ::::::::::::::::::::::::";
            os << std::setprecision(static_cast<int>(precision));
            return os;
        }

        void toJSON(nlohmann::json &jsonMap) const {
            jsonMap["mean"] = mean;
            jsonMap["p5th"] = p5th;
            jsonMap["p95th"] = p95th;
            jsonMap["samples"] = runs;
        }
    };

/**
 * @brief Function to find the minimum value in a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @return The minimum value.
 */
    template<typename T>
    T min(const std::vector<T> &v) {
        if (v.empty()) {
            return NAN;
        }
        return *std::min_element(v.begin(), v.end());
    }

/**
 * @brief Function to find the maximum value in a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @return The maximum value.
 */
    template<typename T>
    T max(const std::vector<T> &v) {
        if (v.empty()) {
            return std::numeric_limits<T>::quiet_NaN();
        }
        return *std::max_element(v.begin(), v.end());
    }

/**
 * @brief Function to find the sum of all elements in a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @return The sum of all elements.
 */
    template<typename T>
    T sum(const std::vector<T> &v) {
        auto data = v.data();
        return std::accumulate(data.begin(), data.end(), static_cast<T>(0));
    }

/**
 * @brief Function to find the mean value of a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @return The mean value.
 */
    template<typename T>
    T mean(const std::vector<T> &v) {
        return Canopy::Sampler::Stats::sum(v) / static_cast<T>(v.data().size());
    }

/**
 * @brief Function to find the variance of a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @return The variance.
 */
    template<typename T>
    T variance(const std::vector<T> &v) {
        T m = Canopy::Sampler::Stats::mean(v);
        T sum = static_cast<T>(0);
        auto data = v.data();
        std::for_each(data.begin(), data.end(), [&](long double x) { sum += (x - m) * (x - m); });
        return sum / data.size();
    }

/**
 * @brief Function to find the standard deviation of a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @return The standard deviation.
 */
    template<typename T>
    T std(const std::vector<T> &v) {
        return std::sqrt(Canopy::Sampler::Stats::variance(v));
    }

/**
 * @brief Function to find the p-th percentile of a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @param p The percentile to find.
 * @return The p-th percentile.
 */
    template<typename T>
    T percentile(std::vector<T> &v, long double p) {
        auto data = v.data();
        if (p < 0 || p > 100) {
            throw std::invalid_argument("Percentile must be between 0 and 100");
        }
        long double rank = (p / static_cast<long double>(100)) * (data.size() - static_cast<long double>(1));
        auto index = static_cast<size_t>(rank);
        long double fractional = rank - index;
        if (data.size() == 1) {
            return data[0];
        }
        std::nth_element(data.begin(), data.begin() + (index), data.end());
        T val = data[index];
        if (fractional == 0 || index + 1 == data.size()) {
            return val;
        }
        std::nth_element(data.begin(), data.begin() + index + 1, data.end());
        return val + fractional * (data[index + 1] - val);
    }

/**
 * @brief Function to find the median of a vector.
 * @tparam T The type of the elements in the vector.
 * @param v The vector.
 * @return The median.
 */
    template<typename T>
    T median(std::vector<T> &v) {
        return percentile(v, 50);
    }

}

#endif //CANOPY_STATS_H
