#ifndef CANOPY_UTILS_STATS_H
#define CANOPY_UTILS_STATS_H

#include <algorithm>
#include <cmath>
#include <numeric>

#include <boost/math/distributions/normal.hpp>
#include <utility>
#include <stdexcept>

/**
 * @namespace canopy::utils
 * @brief A namespace for statistics of boolean tallies, i.e. tallies where the outcome is 0 or 1. The mean and variance
 * estimators in this case are largely simplified since there is no need to maintain a tally vector or a tally_squared
 * vector. This saves [2 * total_tallies * sizeof(size_type)] bytes of space, which is welcome, but takes away the
 * ability to develop a weighted-tally sampling technique.
 */
namespace canopy::utils {

    template<typename float_type, typename size_type>
    static inline float_type mean(const size_type count, const size_type samples) {
        return (static_cast<float_type>(count) / static_cast<float_type>(samples));
    }

    template<typename float_type>
    static inline float_type absolute_error(const float_type estimated, const float_type known) {
        return std::abs(estimated - known);
    }

    template<typename float_type>
    static inline float_type relative_error(const float_type error, const float_type known) {
        const auto hundred_times_error = static_cast<float_type>(100) * error;
        return (hundred_times_error / known);
    }

    template<typename float_type, typename size_type>
    static inline float_type variance(const float_type mean, const size_type samples) {
        const auto mean_squared = mean * mean;
        return ((mean - mean_squared) / static_cast<float_type>(samples));
    }

    /**
     * @remarks The standard error represents the standard deviation of the sampling distribution of ( \hat{P}(F) ). It
     * quantifies the expected deviation of the sample proportion from the true probability ( P(F) ). In practical
     * terms, it tells us how much we can expect our estimated ( \hat{P}(F) ) to vary from the true ( P(F) ) due to
     * random sampling error.
     *
     * @tparam float_type
     * @tparam size_type
     * @param mean
     * @param samples
     * @return
     */
    template<typename float_type, typename size_type>
    static inline float_type standard_error(const float_type mean, const size_type samples) {
        const auto one_minus_mean = static_cast<float_type>(1) - mean;
        const auto numerator = mean * one_minus_mean;
        const auto term = numerator / static_cast<float_type>(samples);
        const auto square_root = std::sqrt(term);
        return square_root;
    }

    template<typename float_type, typename size_type>
    static std::pair<float_type, float_type> confidence_interval(const float_type mean, const size_type num_samples, const float_type confidence_level) {
        if (num_samples == 0) {
            throw std::invalid_argument("Number of samples must be greater than 0.");
        }

        // Calculate the z-score for the given confidence level
        const boost::math::normal_distribution<float_type> dist(0, 1);
        const float_type z = boost::math::quantile(dist, (1 + confidence_level) / 2);

        // Calculate the standard error
        const float_type standard_error = canopy::utils::standard_error<float_type, size_type>(mean, num_samples);

        // Calculate the margin of error
        float_type margin_of_error = z * standard_error;

        // Calculate the confidence interval limits
        float_type lower_limit = mean - margin_of_error;
        float_type upper_limit = mean + margin_of_error;

        // Ensure limits are within [0, 1]
        lower_limit = std::max(lower_limit, static_cast<float_type>(0.0));
        upper_limit = std::min(upper_limit, static_cast<float_type>(1.0));

        return std::make_pair(lower_limit, upper_limit);
    }

    template<typename float_type, typename size_type>
    class SummaryStatistics {

    public:
        SummaryStatistics(const size_type trueTallies, const size_type totalTallies, const float_type knownProbability = std::numeric_limits<float_type>::quiet_NaN())
                : true_tallies_(trueTallies), total_tallies_(totalTallies), known_probability_(knownProbability) {
            computed_mean_ = canopy::utils::mean<float_type, size_type>(true_tallies_, total_tallies_);
            computed_variance_ = canopy::utils::variance<float_type, size_type>(computed_mean_, total_tallies_);
            computed_absolute_error_ = canopy::utils::absolute_error<float_type>(computed_mean_, known_probability_);
            computed_relative_error_ = canopy::utils::relative_error<float_type>(computed_absolute_error_, known_probability_);
            computed_standard_error_ = canopy::utils::standard_error<float_type, size_type>(computed_mean_, total_tallies_);
            computed_p95_ci_ =  canopy::utils::confidence_interval<float_type, size_type>(computed_mean_, total_tallies_, 0.95);
            computed_p99_ci_ =  canopy::utils::confidence_interval<float_type, size_type>(computed_mean_, total_tallies_, 0.99);
        }

    protected:
        /// provided in constructor
        const size_type true_tallies_;
        const size_type total_tallies_;
        const float_type known_probability_;

        /// computed values
        float_type computed_mean_;
        float_type computed_variance_;
        float_type computed_relative_error_;
        float_type computed_absolute_error_;
        float_type computed_standard_error_;
        std::pair<float_type, float_type> computed_p95_ci_;
        std::pair<float_type, float_type> computed_p99_ci_;

    private:
        /**
         * @brief Overloaded stream insertion operator to print the summary statistics.
         * @param os The output stream.
         * @param summary The summary statistics to print.
         * @return The output stream.
         */
        friend std::ostream &operator<<(std::ostream &os, const SummaryStatistics &summary) {
            auto precision = std::cout.precision() ;
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
}

#endif //CANOPY_UTILS_STATS_H
