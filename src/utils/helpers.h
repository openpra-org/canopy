#pragma once

/**
 * @file helpers.h
 * @author Arjun Earthperson
 * @date 08/30/2023
 * @brief This file contains helper functions and structures used throughout the program.
 */
#include <sys/ioctl.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include <boost/program_options.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

namespace canopy::utils {
/**
 * @brief This function replaces the value of a specified option in a map with a new value.
 *
 * @tparam T The type of the value to be replaced.
 * @param vm A map containing program options and their values.
 * @param opt The option whose value is to be replaced.
 * @param val The new value to replace the old value with.
 *
 * @note This function uses the boost library's any class to allow for type erasure,
 * meaning the function can accept any type for the new value.
 */
template <class T>
void replace(std::map<std::string, boost::program_options::variable_value> &vm, const std::string &opt, const T &val) {
    vm[opt].value() = boost::any(val);
}

/**
 * @brief Synchronizes the values of a given key between a JSON object and a Boost variables_map object.
 *
 * This function first checks if the key exists in the variables_map. If it does, the value is updated
 * in the JSON object. Then, if the key exists in the JSON object, the value is read from the JSON object
 * and updated in the variables_map.
 *
 * @tparam T The type of the value associated with the key.
 * @param key The key to synchronize between the JSON object and the variables_map.
 * @param inputMap The JSON object containing the key-value pairs.
 * @param map The Boost variables_map object containing the key-value pairs.
 */
template <typename T>
void syncMapKeys(std::string &key, nlohmann::json &inputMap, boost::program_options::variables_map &map) {
    // first, check if key is in the variables_map
    if (map.count(key)) {
        // if yes, update it in the JSON map.
        inputMap[key] = map[key].as<T>();
    }
    // then, if key is in the JSON map, read value from the JSON map into the variables_map.
    if (inputMap.contains(key)) {
        replace(map, key, static_cast<T>(inputMap[key]));
    }
}

/**
 * @brief This function fills a vector with evenly spaced values between a start and end value.
 *
 * @tparam T The type of the values to be generated.
 * @param result The vector to be filled with the generated values.
 * @param start The start value of the range.
 * @param end The end value of the range.
 * @param count The number of values to be generated.
 *
 * @note This function uses the std::generate function to generate the values.
 */
template <typename T> void fill_linspace(std::vector<T> &result, T start, T end, size_t count) {
    result.reserve(count);

    if (count <= 1) {
        result.clear();
        result[0] = start;
        return;
    }

    T step = (end - start) / static_cast<T>(count - 1);

    std::generate(result.begin(), result.end(), [&start, step]() mutable {
        T value = start;
        start += step;
        return value;
    });
}

/**
 * @brief Scales a value from one range to another.
 * @tparam T The data type of the value and range limits
 * @param value The value to be scaled
 * @param old_min The lower limit of the original range
 * @param old_max The upper limit of the original range
 * @param new_min The lower limit of the new range
 * @param new_max The upper limit of the new range
 * @return The scaled value in the new range
 */
template <typename T> static T scale(T value, T old_min, T old_max, T new_min, T new_max) {
    return (value - old_min) * (new_max - new_min) / (old_max - old_min) + new_min;
}

/**
 * @brief Calculates the number of iterations for a given point in the Julia set.
 * @tparam T The type of the complex numbers.
 * @param z0 The initial complex number.
 * @param c The constant complex number.
 * @param max_iterations The maximum number of iterations.
 * @return The number of iterations before the point escapes the Julia set.
 */
template <typename T>
size_t juliaSetIterations(const std::complex<T> &z0, const std::complex<T> &c, const size_t &max_iterations) {
    std::complex<T> z = z0;
    size_t iter = 0;
    for (; iter < max_iterations; iter++) {
        z = z * z + c;
        if (std::norm(z) > 4.0)
            break;
    }
    return iter;
}

/**
 * @brief A template structure for ToneMap.
 *
 * This structure is used to map tones using a scaling factor and a growth rate.
 * It is a template structure, so it can be used with any data type.
 *
 * @tparam T This is the type of the scaling factor and the growth rate. It can be any type that supports assignment and
 * arithmetic operations.
 */
template <typename T> struct ToneMapType {
    /**
     * @brief The scaling factor for the tone map.
     *
     * This is the factor by which the input tone will be scaled.
     * It is initialized to a very small value (1e-7) to prevent large initial tones.
     */
    T scaling_factor = 1e-7;

    /**
     * @brief The growth rate for the tone map.
     *
     * This is the rate at which the tone map grows.
     * It is initialized to a moderate value (0.3) to allow for a reasonable growth rate.
     */
    T growth_rate = 0.3;
};
typedef ToneMapType<long double> ToneMap;

/**
 * @brief A struct representing the properties of a canvas for rendering ANSI characters
 * @tparam T The type of the numbers used for the canvas dimensions and properties.
 */
template <typename T> struct CanvasType {
    std::complex<T> c;
    std::pair<T, T> x_bounds = {-2, 2};
    std::pair<T, T> y_bounds = {-2, 2};
};
typedef CanvasType<__float128> Canvas;

template <typename T> struct ViewPortType {
    T width = 80;
    T height = 24;

    [[nodiscard]] ViewPortType() {
       fill();
    }

    inline void fill() {
        winsize viewport{};

        for (int fds[] = {STDOUT_FILENO, STDIN_FILENO, STDERR_FILENO}; const int fd : fds) {
            if (isatty(fd) && ioctl(fd, TIOCGWINSZ, &viewport) == 0) {
                if (viewport.ws_row > 0 && viewport.ws_col > 0) {
                    width = viewport.ws_row;
                    height = viewport.ws_col;
                    return;
                }
            }
        }

        const char* env_lines = std::getenv("LINES");
        const char* env_cols  = std::getenv("COLUMNS");
        char* endptr1 = nullptr;
        char* endptr2 = nullptr;
        errno = 0;
        const long lines = env_lines ? std::strtol(env_lines, &endptr1, 10) : 0;
        const long columns  = env_cols  ? std::strtol(env_cols,  &endptr2, 10) : 0;

        if (errno == 0 && endptr1 && *endptr1 == '\0' && endptr2 && *endptr2 == '\0' &&
            lines > 0 && lines <= INT_MAX && columns > 0 && columns <= INT_MAX) {
            width = static_cast<T>(lines);
            height = static_cast<T>(columns);
            }
    }
};
typedef ViewPortType<std::uint16_t> ViewPort;

/**
 * @tparam T The type of the numbers used for the complex numbers.
 * @param canvas The canvas properties for rendering the Julia set.
 * @param c The complex constant
 * @param max_iterations The maximum number of iterations (default is 120).
*/
template <typename T> struct FractalArtType {
    CanvasType<T> canvas;
    size_t max_iterations = 120;
    std::string character = "█";
    T contrast = 1.0;
    ToneMap tone_map;
    ViewPort viewport;
};
typedef FractalArtType<__float128> FractalArt;

/**
 * @brief Generates a vector of 256-bit ANSI gray shades.
 * @return A vector of ANSI escape codes representing gray shades.
 */
template <typename T = long double>
static std::vector<std::string> getGrays256bitANSI(const ToneMapType<T> &tone_map) {

    constexpr size_t shades = 26;
    auto x = std::vector<T>(shades);
    constexpr T x_min = 232;
    constexpr T x_max = 255;
    fill_linspace(x, x_min, x_max, shades);

    auto y = std::vector<T>(shades);
    auto y_min = std::numeric_limits<T>::max();
    auto y_max = std::numeric_limits<T>::min();
    for (size_t i = 0; i < x.size(); i++) {
        y[i] = tone_map.scaling_factor * std::exp(tone_map.growth_rate * x[i]);
        if (y[i] > y_max) {
            y_max = y[i];
        }
        if (y[i] < y_min) {
            y_min = y[i];
        }
    }


    std::vector<std::string> grayShades;
    grayShades.emplace_back("\033[38;5;016m"); // black

    // 23 shades of gray
    for (size_t i = 0; i < 24; ++i) {
        const auto idx = std::to_string(static_cast<size_t>(std::floor(x_min + i)));
        grayShades.push_back("\033[38;5;" + idx + "m");
    }
    grayShades.emplace_back("\033[38;5;231m"); // white

    std::vector<std::string> toneMap;
    for (size_t i = 0; i <= 25; ++i) {
        auto yi = 0 + (y[i] - y_min) * (25 - 0) / (y_max - y_min);
        auto idx = static_cast<size_t>(std::floor(yi));
        toneMap.push_back(grayShades[idx]);
    }
    return toneMap;
}

/**
 * @brief Generates a vector of 256-bit ANSI HSV colors.
 * @return A vector of ANSI escape codes representing HSV colors.
 */
static std::vector<std::string> getHSV256bitANSI() {
    std::vector<std::string> colors;

    // 1
    colors.emplace_back("\033[38;5;016m"); // black

    // 1 + 24
    for (int i = 232; i < 256; i++) {
        colors.push_back("\033[38;5;" + std::to_string(i) + "m");
    }

    // 1 + 24 + 1
    colors.emplace_back("\033[38;5;231m"); // white

    //
    std::vector<std::string> hues = {
        "225", "219", "218", "211", "212", "213", "207", "206", "200", "201", "165", "129", "093",
        "057", "021", "027", "033", "039", "045", "051", "087", "050", "049", "086", "085", "048",
        "047", "084", "083", "046", "082", "119", "155", "118", "154", "191", "190", "226", "227",
        "221", "220", "214", "208", "202", "160", "196", "197", "198", "197",
    };

    for (auto &hue : hues) {
        colors.push_back("\033[38;5;" + hue + "m");
    }

    return colors;
}

template <typename T>
struct Stats {
    T min, max, mean, median;
};

template <typename T>
Stats<T> compute_stats(const std::vector<T>& data) {
    Stats<T> stats;
    if (data.empty()) {
        stats.min = stats.max = stats.mean = stats.median = T(0);
        return stats;
    }
    stats.min = *std::min_element(data.begin(), data.end());
    stats.max = *std::max_element(data.begin(), data.end());
    stats.mean = std::accumulate(data.begin(), data.end(), T(0)) / T(data.size());

    std::vector<T> sorted = data;
    std::nth_element(sorted.begin(), sorted.begin() + sorted.size()/2, sorted.end());
    if (sorted.size() % 2 == 0) {
        T m1 = *std::max_element(sorted.begin(), sorted.begin() + sorted.size()/2);
        T m2 = *std::min_element(sorted.begin() + sorted.size()/2, sorted.end());
        stats.median = (m1 + m2) / T(2);
    } else {
        stats.median = sorted[sorted.size()/2];
    }
    return stats;
}

/**
 * @brief Prints the Julia set to the console using the specified canvas properties.
 * @tparam T The type of the numbers used for the complex numbers.
 * @param art The canvas properties for rendering the Julia set.
 */
template <typename T = __float128>
void printJuliaSet(FractalArt &art) {
    const auto width = art.viewport.width;
    const auto height = art.viewport.height;
    const auto x_start = art.canvas.x_bounds.first;
    const auto x_stop = art.canvas.x_bounds.second;
    const auto x_range = x_stop - x_start;
    const auto x_step = x_range / static_cast<T>(width);

    const auto y_start = art.canvas.y_bounds.first;
    const auto y_stop = art.canvas.y_bounds.second;
    const auto y_range = y_stop - y_start;
    const auto y_step = y_range / static_cast<T>(height);

    // Prepare storage for log2(iter) values
    std::vector<T> log_iterations(width * height);

#pragma omp parallel default(none) shared(log_iterations, width, height, x_start, y_start, x_step, y_step, art)
    {
        #pragma omp for
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                const auto x0 = x_start + x * x_step;
                const auto y0 = y_start + y * y_step;
                const auto z0 = std::complex<T>(x0, y0);
                const size_t iter = juliaSetIterations(z0, art.canvas.c, art.max_iterations);
                const T log_iter = std::log2(iter);
                log_iterations[y * width + x] = log_iter;
                //std::cout<<"Iteration "<<static_cast<long double>(log_iter)<<"\n";
            }
        }
    }

    // Compute statistics manually
    Stats<T> stats = compute_stats(log_iterations);

    const std::vector<std::string> colors = getGrays256bitANSI(art.tone_map);

    // Prepare output buffer: one string per row
    std::vector<std::string> output(height);

    // Second pass: color mapping and string construction, using precomputed z
    #pragma omp parallel for default(none) shared(art, width, height, x_step, y_step, colors, output, stats, log_iterations)
    for (size_t y = 0; y < height; ++y) {
        std::string line;
        line.reserve(width * (8 + 1)); // Estimate: color code + char
        for (size_t x = 0; x < width; ++x) {
            const T log_iter = log_iterations[y * width + x];
            const T enhanced_value = (log_iter - stats.mean) * art.contrast + art.contrast * stats.mean;
            const T color_index = scale<T>(enhanced_value, stats.min, stats.max, T(0), T(colors.size()));
            const size_t clamped_index = std::clamp<size_t>(static_cast<size_t>(color_index), 0, colors.size() - 1);
            line += colors[clamped_index];
            line += art.character;
        }
        output[y] = std::move(line);
    }

    // Serial output
    for (const auto& line : output) {
        std::cout << line << '\n';
    }
    std::cout << "\033[0m";
    //
    // // Print statistics
    // std::cout << "min: " << static_cast<long double>(stats.min)
    //           << " max: " << static_cast<long double>(stats.max)
    //           << " avg: " << static_cast<long double>(stats.mean)
    //           << " p50: " << static_cast<long double>(stats.median)
    //           << std::endl;
}
/**
 * @brief Draws the Julia set to a string buffer using the specified canvas properties.
 * @tparam T The type of the numbers used for the complex numbers.
 * @param art The canvas properties for rendering the Julia set.
 */
template <typename T = __float128>
std::string drawJuliaSet(FractalArt &art) {
    // Create a string-stream to capture the output
    std::ostringstream output;
    // Redirect the standard output stream (cout) to the stringstream
    std::streambuf* originalCoutBuffer = std::cout.rdbuf(output.rdbuf());
    printJuliaSet<T>(art);
    // Restore the original cout stream buffer
    std::cout.rdbuf(originalCoutBuffer);
    // Extract the captured output as a string
    return output.str();
}

typedef boost::accumulators::tag::mean mean;
typedef boost::accumulators::tag::variance variance;
/**
 * @brief Computes the mean and variance of a numeric vector using Boost Accumulators.
 *
 * This function calculates the mean and variance of a given numeric vector using Boost
 * Accumulators library. It returns an accumulator set pre-configured with mean and variance tags.
 *
 * @tparam T The data type of the numeric vector elements.
 * @param numbers The input numeric vector for which to compute the mean and variance.
 * @return An accumulator set containing computed mean and variance.
 *
 * @note To obtain the actual mean and variance values, you can use the `boost::accumulators::mean`
 * and `boost::accumulators::variance` functions on the returned accumulator set, respectively.
 * For example:
 * ```
 * boost::accumulators::accumulator_set<T, boost::accumulators::features<mean, variance>> acc = computeMeanStd(numbers);
 * double mean = boost::accumulators::mean(acc);
 * double variance = boost::accumulators::variance(acc);
 * ```
 */
template<typename T>
boost::accumulators::accumulator_set<T, boost::accumulators::features<mean, variance>> computeMeanStd(const std::vector<T> &numbers) {
    boost::accumulators::accumulator_set<T, boost::accumulators::features<mean, variance>> accumulator;
    // Iterate through the elements of the input vector and add them to the accumulator.
    for (T number : numbers) {
        accumulator(number);
    }
    return accumulator;
}

/**
 * @brief Formats the mean and standard deviation as a string.
 *
 * This function takes the calculated mean and standard deviation values and formats them
 * as a string in the following format: "mean ± std".
 *
 * @tparam T The data type of the mean and standard deviation values.
 * @param avg The calculated mean value.
 * @param std The calculated standard deviation value.
 * @return A string containing the formatted mean and standard deviation.
 *
 * @note This function is useful for converting mean and standard deviation values into a
 * human-readable format for display or logging.
 *
 * @example
 * ```cpp
 * double calculatedMean = 5.0;
 * double calculatedStdDev = 1.0;
 * std::string formattedResult = formatMeanStd(calculatedMean, calculatedStdDev);
 * // Result: "5.000000 ± 1.000000"
 * ```
 */
template<typename T>
inline std::string formatMeanStd(T avg, T std) {
    return std::to_string(avg) + " ± " + std::to_string(std);
}

/**
 * @brief Checks if a container contains a specific value.
 *
 * @details This function uses the std::find algorithm to check if a specific value is present in a container.
 *
 * @tparam Container The type of the container. It should support begin() and end() methods.
 * @param container The container in which to search for the value.
 * @param value The value to search for in the container.
 *
 * @return True if the value is found in the container, false otherwise.
 */
template <typename Container>
bool contains(const Container& container, const typename Container::value_type& value) {
    return std::find(container.begin(), container.end(), value) != container.end();
}

}
