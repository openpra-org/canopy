/**
* @file Profiler.h
* @author Arjun Earthperson
* @date 10/20/2024
* @brief This file contains the Profiler class which is used to profile the execution time of a function.
*/

#ifndef CANOPY_UTILS_PROFILER_H
#define CANOPY_UTILS_PROFILER_H

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <utility>

#include "profile.h"
#include "stopwatch.h"
#include "stats.h"

namespace canopy::utils {

    /**
     * @brief A template class to profile the execution time of a function.
     * @tparam FunctionType The type of the function to be profiled.
     */
    template<class FunctionType = std::function<void()>>
    class Profiler {
    public:

        /**
         * @brief Constructor for the Profiler class.
         * @param function The function to be profiled.
         * @param runs The number of times the function should be run for profiling.
         * @param timeout The maximum time allowed for the function to run.
         * @param description A description of the function being profiled.
         */
        explicit Profiler(FunctionType function, size_t runs = 1, long double timeout = 0, std::string description = "")
                : _function(function) {
            _timedOut = false;
            _totalRuns = runs > 0 ? runs : 1;
            _timeout = timeout > 0 ? timeout : 0;
            _stopwatches = std::vector<Stopwatch<Nanoseconds>>(0);
            _description = std::move(description);
            _summary = canopy::utils::Profile<long double>();
        }

        /**
         * @brief Default constructor for the Profiler class.
         * @details This constructor initializes a Profiler object without any function to profile.
         *          The function to profile can be set later using the assignment operator.
         */
        Profiler() = default;

        /**
         * @brief Gets the total number of runs for profiling.
         * @return The total number of runs for profiling.
         */
        [[nodiscard]] size_t get_total_runs() const { return _totalRuns; }

        /**
         * @brief Runs the function for profiling and summarizes the results.
         * @return A reference to this Profiler object.
         */
        Profiler &run() {
            reset_runs();
            if (_timeout > 0) {
                _timedOut = run_with_timeout();
            } else {
                run_without_timeout();
                _timedOut = false;
            }
            summarize(_summary, _stopwatches);
            return *this;
        }

        /**
         * @brief Overloads the assignment operator to set the function to be profiled.
         * @param function The function to be profiled.
         * @return A reference to this Profiler object.
         */
        Profiler &operator=(const std::function<void()> &function) {
            this->_function = function;
            return *this;
        }

        /**
         * @brief Overloads the << operator to print the summary of the profiling.
         * @param os The output stream to print to.
         * @param m The Profiler object to print.
         * @return The output stream.
         */
        friend std::ostream &operator<<(std::ostream &os, const Profiler &m) {
            os << R"(:::::::::::::::::::::::::::: PROFILER SUMMARY [ns] ::::::::::::::::::::::::::::::)" << std::endl;
            os << "runs: [" << m._stopwatches.size() << "] : " << m._description << std::endl;
            os << R"(:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::)" << std::endl;
            os << m._summary;
            os << R"(:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::)" << std::endl;
            return os;
        }

        /**
         * @brief Gets the summary of the profiling.
         * @return The summary of the profiling.
         */
        const canopy::utils::Profile<long double> &summary() {
            _summary.runs = _stopwatches.size();
            return _summary;
        }

    private:
        FunctionType _function; ///< The function to be profiled.
        size_t _totalRuns{}; ///< The number of times the function should be run for profiling.
        long double _timeout{}; ///< The maximum time allowed for the function to run.
        bool _timedOut{}; ///< Whether the function timed out during profiling.
        std::string _description; ///< A description of the function being profiled.
        std::vector<Stopwatch<Nanoseconds>> _stopwatches; ///< A vector of stopwatches to time each run of the function.
        canopy::utils::Profile<long double> _summary; ///< The summary of the profiling.

        /**
         * @brief Runs the function for profiling without a timeout.
         */
        void run_without_timeout() {
            for (size_t i = 0; i < _totalRuns; i++) {
                auto stopwatch = Stopwatch<Nanoseconds>().restart();
                _function();
                stopwatch.click();
                _stopwatches.emplace_back(stopwatch);
            }
        }

        /**
         * @brief Runs the function for profiling with a timeout.
         * @return Whether the function timed out during profiling.
         */
        bool run_with_timeout() {
            auto timeoutWatch = Stopwatch<Nanoseconds>();
            timeoutWatch.restart();

            for (size_t i = 0; i < _totalRuns; i++) {
                // timed out
                if (timeoutWatch.peek_elapsed_time().count() > _timeout) {
                    _timedOut = true;
                    break;
                }
                // run the calculation
                auto stopwatch = Stopwatch<Nanoseconds>().restart();
                _function();
                stopwatch.click();
                _stopwatches.emplace_back(stopwatch);
            }

            return _timedOut;
        }

        /**
         * @brief Resets the runs for profiling.
         */
        void reset_runs() {
            _timedOut = false;
            _stopwatches.clear();
            _stopwatches.resize(0);
            _summary = canopy::utils::Profile<long double>();
        }

        /**
         * @brief Summarizes the results of the profiling.
         * @param summary The summary to store the results in.
         * @param stopwatches The stopwatches used to time each run of the function.
         */
        static void summarize(canopy::utils::Profile<long double> &summary,
                              const std::vector<Stopwatch<Nanoseconds>> &stopwatches) {

            // Create an array of durations
            std::vector<long double> durations(stopwatches.size());
            std::transform(stopwatches.begin(), stopwatches.end(), durations.begin(),
                           [](const Stopwatch<Nanoseconds> &stopwatch) { return stopwatch.duration().count(); });

            // Sort the durations in-place
            std::ranges::sort(durations);
            const auto n = durations.size();

            // Calculate statistics
            summary.min = *std::ranges::min_element(durations);
            summary.max = *std::ranges::max_element(durations);
            summary.sum = std::accumulate(durations.begin(), durations.end(), 0.0);
            summary.mean = summary.sum / static_cast<long double>(n);
            summary.variance = std::accumulate(durations.begin(), durations.end(), 0.0,
                                               [mean = summary.mean](long double acc, long double duration) {
                                                   const auto diff = duration - mean;
                                                   return acc + diff * diff;
                                               }) / static_cast<long double>(n);
            summary.stddev = std::sqrt(summary.variance);

            summary.p5th  = durations[static_cast<size_t>(0.05 * n)];
            summary.p95th = durations[static_cast<size_t>(0.95 * n)];
        }
    };

}
#endif // CANOPY_UTILS_PROFILER_H