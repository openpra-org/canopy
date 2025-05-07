#ifndef CANOPY_SAMPLER_IMPORTANCE_H
#define CANOPY_SAMPLER_IMPORTANCE_H
#include "utils/types.h"

/**
 * @file importance.h
 * @author Arjun Earthperson
 * @date 10/21/2024
 * @brief This file contains methods for importance sampling.
 */

static void sample_and_assign_truth_values_IS(
        const known_event_probabilities &Qx,
        std::vector<bit_vector_type> &sampled_x,
        std::vector<double> &likelihood_ratios,
        const known_event_probabilities &Px,
        const std::size_t seed = 372)
{
    std::mt19937 stream(seed);
    std::uniform_real_distribution<sampling_distribution_type> uniform(0, 1);

    std::generate(sampled_x.begin(), sampled_x.end(), [&]() {
        bit_vector_type sample = 0b00000000;
        double L = 1.0; // Initialize likelihood ratio for this sample
        for(size_t o = 0; o < s_symbols; o++) {
            double rand_val = uniform(stream);
            bool event = rand_val < Qx[o];
            // Update the sample bit vector
            size_t shift_pos = 2 * o + !event;
            sample |= (0b10000000 >> shift_pos);
            // Update the likelihood ratio
            double P_event = event ? Px[o] : (1.0 - Px[o]);
            double Q_event = event ? Qx[o] : (1.0 - Qx[o]);
            L *= P_event / Q_event;
        }
        likelihood_ratios.push_back(L);
        return sample;
    });
}

#endif //CANOPY_SAMPLER_IMPORTANCE_H
