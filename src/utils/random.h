/**
 * @file random.h
 * @author Arjun Earthperson
 * @date 10/20/2024
 * @brief This file contains the canopy::utils::random namespace which provides functions for generating random vectors
 * and matrices.
 */

#ifndef CANOPY_UTILS_RANDOM_H
#define CANOPY_UTILS_RANDOM_H

#include <algorithm>
#include <random>
#include <vector>
#include <memory>

/**
 * @namespace canopy::utils::random
 * @brief This namespace provides functions for generating random vectors and matrices.
 */
namespace canopy::utils::random {

    /**
     * @brief Generates a single random number.
     * @tparam T The type of the random number.
     * @param min The minimum value for the random number.
     * @param max The maximum value for the random number.
     * @param seed The seed for the random number generator.
     * @return A random number.
     */
    template <typename T>
    static T generate_one(T min = 0, T max = 1, const size_t seed = 372) {
        if (max < min) {
            auto temp = max;
            max = min;
            min = temp;
        }
        std::random_device rd;
        std::mt19937 stream(seed);
        std::uniform_real_distribution<T> uniform(min, max);
        return uniform(stream);
    }

    /**
     * @brief Generates a vector of random numbers.
     * @tparam T The type of the elements in the vector.
     * @param n The size of the vector.
     * @param min The minimum value for the random numbers.
     * @param max The maximum value for the random numbers.
     * @param seed The seed for the random number generator.
     * @return A vector of random numbers.
     */
    template <typename T>
    static std::vector<T> generate_vector(const size_t n, T min = 0, T max = 1, const size_t seed = 372) {
        if (max < min) {
            std::swap(min, max);
        }
        std::random_device rd;
        std::mt19937 stream(seed);
        std::uniform_real_distribution<T> uniform(min, max);
        std::vector<T> samples(n);

        // Use std::generate to fill the vector with random numbers
        std::generate(samples.begin(), samples.end(), [&]() { return uniform(stream); });

        return samples;
    }

} // namespace Random
#endif // CANOPY_UTILS_RANDOM_H
