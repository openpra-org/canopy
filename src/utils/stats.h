#ifndef CANOPY_UTILS_STATS_H
#define CANOPY_UTILS_STATS_H

/**
* @file stats.h
* @author Arjun Earthperson
* @date 10/20/2024
* @brief This file contains the statistical utility functions for estimating the mean, variance, and associated
* confidence intervals.
*/

#include <algorithm>
#include <cmath>
#include <numeric>
#include <boost/math/distributions/normal.hpp>
#include <utility>
#include <stdexcept>
#include <limits>
#include <iostream>
#include <iomanip>

/**
 * @namespace canopy::utils
 * @brief A namespace for statistics of boolean tallies, i.e., tallies where the outcome is 0 or 1.
 *
 * The mean and variance estimators in this case are largely simplified since there is no need to
 * maintain a tally vector or a tally_squared vector. This saves [2 * total_tallies * sizeof(size_type)]
 * bytes of space, which is welcome, but takes away the ability to develop a weighted-tally sampling
 * technique.
 *
 * @example
 * ```cpp
 * size_type true_counts = 500;
 * size_type total_counts = 1000;
 * auto mean_val = canopy::utils::mean<double, size_type>(true_counts, total_counts);
 * ```
 */
namespace canopy::utils {

    /**
     * @brief Calculates the mean of boolean tallies.
     *
     * @tparam float_type Floating-point type for the mean calculation.
     * @tparam size_type Integer type for the tally counts.
     * @param count The number of true tallies (outcome = 1).
     * @param samples The total number of samples.
     * @return The mean value as a floating-point number.
     *
     * @example
     * ```cpp
     * double mean_val = canopy::utils::mean<double, size_type>(true_counts, total_counts);
     * ```
     */
    template<typename float_type, typename size_type>
    static inline float_type mean(const size_type count, const size_type samples) {
        return (static_cast<float_type>(count) / static_cast<float_type>(samples));
    }

    /**
     * @brief Calculates the absolute error between an estimated value and a known value.
     *
     * @tparam float_type Floating-point type for the error calculation.
     * @param estimated The estimated value.
     * @param known The known true value.
     * @return The absolute error as a floating-point number.
     *
     * @example
     * ```cpp
     * double abs_error = canopy::utils::absolute_error<double>(estimated_mean, true_mean);
     * ```
     */
    template<typename float_type>
    static inline float_type absolute_error(const float_type estimated, const float_type known) {
        return std::abs(estimated - known);
    }

    /**
     * @brief Calculates the relative error given the absolute error and the known value.
     *
     * @tparam float_type Floating-point type for the error calculation.
     * @param error The absolute error.
     * @param known The known true value.
     * @return The relative error as a percentage.
     *
     * @example
     * ```cpp
     * double rel_error = canopy::utils::relative_error<double>(abs_error, true_mean);
     * ```
     */
    template<typename float_type>
    static inline float_type relative_error(const float_type error, const float_type known) {
        const auto hundred_times_error = static_cast<float_type>(100) * error;
        return (hundred_times_error / known);
    }

    /**
     * @brief Calculates the variance of boolean tallies.
     *
     * @tparam float_type Floating-point type for the variance calculation.
     * @tparam size_type Integer type for the tally counts.
     * @param mean The mean of the tallies.
     * @param samples The total number of samples.
     * @return The variance as a floating-point number.
     *
     * @example
     * ```cpp
     * double var = canopy::utils::variance<double, size_type>(mean_val, total_counts);
     * ```
     */
    template<typename float_type, typename size_type>
    static inline float_type variance(const float_type mean, const size_type samples) {
        const auto mean_squared = mean * mean;
        return ((mean - mean_squared) / static_cast<float_type>(samples));
    }

    /**
     * @brief Computes the standard error of the mean for boolean tallies.
     *
     * The standard error represents the standard deviation of the sampling distribution of ( \f$ \hat{P}(F) \f$ ).
     * It quantifies the expected deviation of the sample proportion from the true probability ( \f$ P(F) \f$ ).
     * In practical terms, it tells us how much we can expect our estimated ( \f$ \hat{P}(F) \f$ ) to vary from
     * the true ( \f$ P(F) \f$ ) due to random sampling error.
     *
     * @tparam float_type Floating-point type for the standard error calculation.
     * @tparam size_type Integer type for the tally counts.
     * @param mean The mean of the tallies.
     * @param samples The total number of samples.
     * @return The standard error as a floating-point number.
     *
     * @example
     * ```cpp
     * double std_err = canopy::utils::standard_error<double, size_type>(mean_val, total_counts);
     * ```
     */
    template<typename float_type, typename size_type>
    static inline float_type standard_error(const float_type mean, const size_type samples) {
        const auto one_minus_mean = static_cast<float_type>(1) - mean;
        const auto numerator = mean * one_minus_mean;
        const auto term = numerator / static_cast<float_type>(samples);
        const auto square_root = std::sqrt(term);
        return square_root;
    }

    /**
     * @brief Calculates the confidence interval for the mean of boolean tallies.
     *
     * @tparam float_type Floating-point type for the confidence interval calculation.
     * @tparam size_type Integer type for the tally counts.
     * @param mean The mean of the tallies.
     * @param num_samples The total number of samples.
     * @param confidence_level The desired confidence level (e.g., 0.95 for 95% confidence).
     * @return A pair containing the lower and upper bounds of the confidence interval.
     *
     * @throws std::invalid_argument If the number of samples is zero.
     *
     * @example
     * ```cpp
     * auto ci = canopy::utils::confidence_interval<double, size_type>(mean_val, total_counts, 0.95);
     * std::cout << "95% Confidence Interval: " << ci.first << " - " << ci.second << std::endl;
     * ```
     */
    template<typename float_type, typename size_type>
    static std::pair<float_type, float_type> confidence_interval(const float_type mean, const size_type num_samples, const float_type confidence_level) {
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

    /**
     * @class SummaryStatistics
     * @brief Represents a summary of statistical metrics for boolean tallies.
     *
     * This class computes and stores various statistical measures such as mean, variance, absolute and
     * relative errors, standard error, and confidence intervals based on the provided tally counts.
     *
     * @tparam float_type Floating-point type for statistical calculations.
     * @tparam size_type Integer type for tally counts.
     *
     * @example
     * ```cpp
     * size_type true_counts = 500;
     * size_type total_counts = 1000;
     * double known_prob = 0.5;
     *
     * SummaryStatistics<double, size_type> stats(true_counts, total_counts, known_prob);
     * std::cout << stats;
     * ```
     */
    template<typename float_type, typename size_type>
    class SummaryStatistics {
    public:
        /**
         * @brief Constructs a SummaryStatistics object with the given tallies and known probability.
         *
         * @param trueTallies The number of true tallies (outcome = 1).
         * @param totalTallies The total number of tallies (samples).
         * @param knownProbability The known true probability (optional; defaults to NaN).
         *
         * @example
         * ```cpp
         * SummaryStatistics<double, size_type> stats(true_counts, total_counts, known_prob);
         * ```
         */
        SummaryStatistics(const size_type trueTallies, const size_type totalTallies, const float_type knownProbability = std::numeric_limits<float_type>::quiet_NaN())
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

    protected:
        /// The number of true tallies (outcome = 1) provided in the constructor.
        const size_type true_tallies_;

        /// The total number of tallies (samples) provided in the constructor.
        const size_type total_tallies_;

        /// The known true probability provided in the constructor.
        const float_type known_probability_;

        /// The computed mean of the tallies.
        float_type computed_mean_;

        /// The computed variance of the tallies.
        float_type computed_variance_;

        /// The computed relative error of the mean.
        float_type computed_relative_error_;

        /// The computed absolute error of the mean.
        float_type computed_absolute_error_;

        /// The computed standard error of the mean.
        float_type computed_standard_error_;

        /// The computed 95% confidence interval for the mean.
        std::pair<float_type, float_type> computed_p95_ci_;

        /// The computed 99% confidence interval for the mean.
        std::pair<float_type, float_type> computed_p99_ci_;

    private:
        /**
         * @brief Overloaded stream insertion operator to print the summary statistics.
         *
         * @param os The output stream.
         * @param summary The SummaryStatistics object to print.
         * @return The output stream with the appended summary statistics.
         *
         * @example
         * ```cpp
         * std::cout << stats;
         * ```
         */
        friend std::ostream& operator<<(std::ostream& os, const SummaryStatistics& summary) {
            auto precision = os.precision();
            const auto width = static_cast<int>(2 + static_cast<std::streamsize>(10));
            os << std::setprecision(15) << std::setw(width) << std::setfill(' ') << std::scientific;
            os << "total simulations         [n]: "<<summary.total_tallies_<<std::endl;
            os << "true evaluations          [T]: "<<summary.true_tallies_<<std::endl;
            os << "false evaluations         [F]: "<<(summary.total_tallies_-summary.true_tallies_)<<std::endl;
            os << "known expected value P(f) [μ]: "<<summary.known_probability_<<std::endl;
            os << "estimated mean            [m]: "<<summary.computed_mean_<<std::endl;
            os << "estimated variance        [v]: "<<summary.computed_variance_<<std::endl;
            os << "std. error √[m•(1-m)/n] = [s]: "<<summary.computed_standard_error_<<std::endl;
            os << "absolute error  |μ – m| = [ε]: "<<summary.computed_absolute_error_<<std::endl;
            os << "percentage error (ε/μ)% = [%]: "<<summary.computed_relative_error_<<std::endl;
            os << "95% CI: "<<summary.computed_p95_ci_.first<<", "<<summary.computed_p95_ci_.second<<std::endl;
            os << "99% CI: "<<summary.computed_p99_ci_.first<<", "<<summary.computed_p99_ci_.second<<std::endl;
            os << std::setprecision(static_cast<int>(precision));
            os << std::endl;
            return os;
        }
    };

} // namespace canopy::utils

#endif // CANOPY_UTILS_STATS_H
