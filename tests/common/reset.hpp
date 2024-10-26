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

#ifndef CANOPY_HIPSYCL_TESTS_RESET_HPP
#define CANOPY_HIPSYCL_TESTS_RESET_HPP

/**
 * @brief Fixture for resetting the device state before each test.
 */
struct reset_device_fixture {
    /**
     * @brief Sets up the device fixture.
     */
    reset_device_fixture() {
        //reset_device();
    }

    /**
     * @brief Tears down the device fixture.
     */
    ~reset_device_fixture() {
        //reset_device();
    }
};

#endif // CANOPY_HIPSYCL_TESTS_RESET_HPP
