/*
 * This file is part of AdaptiveCpp, an implementation of SYCL and C++ standard
 * parallelism for CPUs and GPUs.
 *
 * Copyright The AdaptiveCpp Contributors
 *
 * AdaptiveCpp is released under the BSD 2-Clause "Simplified" License.
 * See file LICENSE in the project root for full license details.
 */
// SPDX-License-Identifier: BSD-2-Clause
#ifndef BRUTEFORCE_NBODY_H
#define BRUTEFORCE_NBODY_H

#include <CL/sycl.hpp>
using namespace cl;

/**
 * @file bruteforce_nbody.h
 * @brief Header file for the brute force N-body simulation using SYCL.
 */

/**
 * @typedef arithmetic_type
 * @brief Defines the type used for arithmetic operations in the simulation.
 */
using arithmetic_type = float;

/**
 * @typedef vector_type
 * @brief Defines a 3-component vector type used for representing positions and velocities.
 */
using vector_type = sycl::vec<arithmetic_type, 3>;

/**
 * @typedef particle_type
 * @brief Defines a 4-component vector type used for representing particles, including mass.
 */
using particle_type = sycl::vec<arithmetic_type, 4>;

/**
 * @brief Total mass of the system.
 */
constexpr arithmetic_type total_mass = 100000.f;

/**
 * @brief Number of particles in the simulation.
 */
constexpr std::size_t num_particles = 40000;

/**
 * @brief Number of timesteps for the simulation.
 */
constexpr std::size_t num_timesteps = 600;

/**
 * @brief Local size for parallel computation.
 */
constexpr std::size_t local_size = 128;

/**
 * @brief Gravitational softening parameter to prevent singularities.
 */
constexpr arithmetic_type gravitational_softening = 1.e-4f;

/**
 * @brief Time step size for the simulation.
 */
constexpr arithmetic_type dt = 0.1f;

/**
 * @brief Size of the simulation cube.
 */
constexpr arithmetic_type cube_size = 400.0f;

/**
 * @brief Half the size of the simulation cube, used for boundary conditions.
 */
constexpr arithmetic_type half_cube_size = 0.5f * cube_size;

#endif // BRUTEFORCE_NBODY_H
