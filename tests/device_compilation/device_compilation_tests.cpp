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

// Test workaround for https://bugs.llvm.org/show_bug.cgi?id=50383
#include <atomic>
#include <complex>

#define BOOST_TEST_MODULE device compilation tests
#define BOOST_TEST_DYN_LINK

#include <boost/test/included/unit_test.hpp>
#include <initializer_list>
#include <CL/sycl.hpp>
#include "tests/common/reset.hpp"

/**
 * @file device_compilation_tests.cpp
 * @brief Unit tests for device compilation using SYCL and Boost.Test.
 */

// NOTE: Since these tests are concerned with device compilation, the first
// and arguably most important "test" is that this file compiles at all. We
// then also include a few unit tests to assert the correct behavior of the
// resulting programs.

/**
 * @brief Test suite for device compilation.
 */
BOOST_FIXTURE_TEST_SUITE(device_compilation_test_suite, reset_device_fixture)

    /**
     * @brief Adds five to the given integer value.
     *
     * @param v The integer value to add five to.
     * @return The result of adding five to v.
     *
     * @example
     * @code
     * int result = add_five(10); // result is 15
     * @endcode
     */
    int add_five(int v) {
        return v + 5;
    }

    /**
     * @brief Calls a lambda function that modifies the input integer by adding five.
     *
     * @param v Reference to the integer to be modified.
     *
     * @example
     * @code
     * int value = 10;
     * call_lambda(value); // value becomes 15
     * @endcode
     */
    void call_lambda(int &v) {
        ([&]() { v = add_five(v); })();
    }

    /**
     * @brief Represents a parent structure that initializes with a list of integers
     *        and modifies the parent value.
     */
    struct MyParent {
        /**
         * @brief Constructs a MyParent object with an initializer list.
         *
         * @param init List of integers to initialize the parent value.
         *
         * @example
         * @code
         * MyParent parent{1, 2, 3};
         * // parent_value becomes 6, then call_lambda modifies it to 11
         * @endcode
         */
        MyParent(std::initializer_list<int> init) {
            for (auto &&e: init) {
                parent_value += e;
            }
            call_lambda(parent_value);
        }

        /**
         * @brief Virtual destructor that increments a value upon destruction.
         */
        virtual ~MyParent() {
            *increment_in_dtor += parent_value + 7;
        };

        /**
         * @brief The accumulated value from the initializer list and lambda call.
         */
        int parent_value = 0;

        /**
         * @brief Pointer to an integer that will be incremented in the destructor.
         */
        int *increment_in_dtor;
    };

    /**
     * @brief Conversion structure that allows conversion from float to int.
     */
    struct Conversion {
        /**
         * @brief Constructs a Conversion object with a float value.
         *
         * @param value The float value to store.
         */
        Conversion(float value) : value(value) {}

        /**
         * @brief Converts the stored float value to an integer.
         *
         * @return The integer representation of the stored float value.
         */
        operator int() {
            return static_cast<int>(value);
        }

        /**
         * @brief The stored float value.
         */
        float value;
    };

    /**
     * @brief Converts a Conversion object to an integer and adds five.
     *
     * @param conv The Conversion object to convert.
     * @return The result of add_five after conversion.
     *
     * @example
     * @code
     * Conversion conv(10.5f);
     * int result = convert(conv); // result is 15
     * @endcode
     */
    int convert(Conversion conv) {
        return add_five(conv);
    }

/**
 * @brief Derived structure from MyParent that modifies its value during destruction.
 */
    struct MyStruct : MyParent {
        /**
         * @brief Constructs a MyStruct object with an integer reference.
         *
         * @param value Reference to an integer to be modified.
         *
         * @example
         * @code
         * int val = 10;
         * MyStruct obj(val); // parent_value is initialized, val is set to increment_in_dtor
         * @endcode
         */
        MyStruct(int &value) : MyParent::MyParent{value, 4, 5, 7}, value(value) {
            increment_in_dtor = &value;
        }

        /**
         * @brief Destructor that converts the stored value.
         */
        ~MyStruct() {
            value = convert(static_cast<float>(value));
        }

        /**
         * @brief Reference to the integer value being modified.
         */
        int &value;
    };

    /**
     * @brief Template function that creates an instance of type T with U.
     *
     * @tparam T The type to be instantiated.
     * @tparam U The type of the parameter.
     * @param u The parameter to pass to the constructor of T.
     *
     * @example
     * @code
     * MyStruct obj;
     * template_fn<MyStruct>(obj);
     * @endcode
     */
    template<typename T, typename U>
    void template_fn(U &u) {
        T t(u);
    }

    /**
     * @brief Function that utilizes the template_fn with MyStruct.
     *
     * @param value The integer value to process.
     * @return The processed float value.
     *
     * @example
     * @code
     * float result = some_fn(10); // Calls template_fn<MyStruct>(10)
     * @endcode
     */
    float some_fn(int value) {
        template_fn<MyStruct>(value);
        return value;
    }

    /**
     * @remarks The following tests verify different forms of function calls, including implicit and templated functions
     * and constructors as well as destructors. The AdaptiveCpp Clang plugin must recognize that each of these is being
     * called by the user-provided kernel functor, and mark them as __device__ functions accordingly.
     */

    /**
     * @brief Test case for complex device call graphs.
     *
     * This test ensures that various forms of function calls within a SYCL kernel
     * are correctly identified and marked for device compilation.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(complex_device_call_graph)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    BOOST_AUTO_TEST_CASE(complex_device_call_graph) {
        cl::sycl::queue queue;
        cl::sycl::buffer<float, 1> buf(1);
        queue.submit([&](cl::sycl::handler &cgh) {
            auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
            cgh.single_task<class some_kernel>([=]() {
                acc[0] = some_fn(2);
            });
        });
        auto acc = buf.get_access<cl::sycl::access::mode::read>();
        BOOST_REQUIRE(acc[0] == 37);
    }

    /**
     * @brief Template class for complex kernel naming.
     *
     * @tparam T Integer template parameter.
     * @tparam I Pointer to an integer template parameter.
     * @tparam U Type template parameter.
     */
    template<int T, int *I, typename U>
    class complex_kn;

    /**
     * @brief Template class for templated kernel naming.
     *
     * @tparam T Type template parameter.
     */
    template<typename T>
    class templated_kn;

    /**
     * @brief Enumeration for testing enum-based kernel naming.
     */
    enum class my_enum {
        HELLO = 42, ///< Represents the "HELLO" state.
        WORLD       ///< Represents the "WORLD" state.
    };

    /**
     * @brief Template class for enum-based kernel naming.
     *
     * @tparam E Enum value from my_enum.
     */
    template<my_enum E>
    class enum_kn;

    /**
     * @brief Template class for templated template kernel naming.
     *
     * @tparam T Template template parameter.
     */
    template<template<typename, int, typename> class T>
    class templated_template_kn;

    /**
     * @brief Test case for kernel name mangling.
     *
     * This test verifies that different kernel names with complex template
     * parameters are correctly mangled and do not collide.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(kernel_name_mangling)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    BOOST_AUTO_TEST_CASE(kernel_name_mangling) {
        cl::sycl::queue queue;

        // Qualified / modified types
        queue.submit([&](cl::sycl::handler &cgh) {
            cgh.single_task<templated_kn<const unsigned int>>([]() {});
        });

        queue.submit([&](cl::sycl::handler &cgh) {
            cgh.single_task<templated_kn<complex_kn<32, nullptr, enum_kn<my_enum::HELLO>>>>([]() {});
        });

        queue.submit([&](cl::sycl::handler &cgh) {
            cgh.single_task<templated_template_kn<cl::sycl::buffer>>([]() {});
        });

        // No collision happens between the following two names
        queue.submit([&](cl::sycl::handler &cgh) {
            cgh.single_task<templated_kn<class collision>>([]() {});
        });

        queue.submit([&](cl::sycl::handler &cgh) {
            cgh.single_task<class templated_kn_collision>([]() {});
        });
    }

    /**
     * @brief Functor for kernel execution that modifies a buffer using an accessor.
     *
     * @tparam AccessorT The type of the SYCL accessor.
     */
    template<class AccessorT>
    struct KernelFunctor {
        /**
         * @brief Constructs the functor with the given accessor.
         *
         * @param acc The SYCL accessor to the buffer.
         *
         * @example
         * @code
         * KernelFunctor<AccessorType> functor(acc);
         * @endcode
         */
        KernelFunctor(AccessorT acc) : acc(acc) {}

        /**
         * @brief Overloads the function call operator to execute within the kernel.
         *
         * @param item The SYCL item representing the work-item.
         *
         * @example
         * @code
         * KernelFunctor functor(acc);
         * queue.submit([&](cl::sycl::handler& cgh){
         *     cgh.parallel_for(range, functor);
         * });
         * @endcode
         */
        void operator()(cl::sycl::item<1> item) const {
            acc[0] = 300 + item.get_linear_id();
        }

        /**
         * @brief The SYCL accessor to the buffer.
         */
        AccessorT acc;
    };

    /**
     * @brief Test case for omitting kernel names when the functor is globally visible.
     *
     * This test verifies that unnamed kernels are correctly handled by the SYCL
     * compiler when the functor is globally visible.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(omit_kernel_name)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    BOOST_AUTO_TEST_CASE(omit_kernel_name) {
        cl::sycl::queue queue;
        cl::sycl::buffer<size_t, 1> buf(1);

        {
            queue.submit([&](cl::sycl::handler &cgh) {
                auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
                cgh.parallel_for(cl::sycl::range<1>(1), KernelFunctor(acc));
            });
            auto acc = buf.get_access<cl::sycl::access::mode::read>();
            BOOST_REQUIRE(acc[0] == 300);
        }

        {
            queue.submit([&](cl::sycl::handler &cgh) {
                auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
                cgh.parallel_for(cl::sycl::range<1>(1), cl::sycl::id<1>(1), KernelFunctor(acc));
            });
            auto acc = buf.get_access<cl::sycl::access::mode::read>();
            BOOST_REQUIRE(acc[0] == 301);
        }
    }

    /**
     * @brief Test case for hierarchical invocation with shared memory.
     *
     * This test checks the correct handling of hierarchical parallelism and shared
     * memory within SYCL kernels.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(hierarchical_invoke_shared_memory)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    //  BOOST_AUTO_TEST_CASE(hierarchical_invoke_shared_memory) {
    //     cl::sycl::queue queue;
    //
    //     // The basic case, as outlined in the SYCL spec.
    //     {
    //         cl::sycl::buffer<size_t, 1> buf(4);
    //         queue.submit([&](cl::sycl::handler& cgh) {
    //             auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
    //             cgh.parallel_for_work_group<class shmem_one>(
    //                     cl::sycl::range<1>(2),
    //                     cl::sycl::range<1>(2),
    //                     [=](cl::sycl::group<1> group) {
    //                         {
    //                             // Do this in a block to ensure correct handling by Clang plugin.
    //                             size_t shmem[2]; ///< Shared memory array.
    //
    //                             // Populate shared memory with global linear IDs.
    //                             group.parallel_for_work_item([&](cl::sycl::h_item<1> h_item) {
    //                                 shmem[h_item.get_local().get_linear_id()] = h_item.get_global().get_linear_id();
    //                             });
    //
    //                             // Read from shared memory and write to buffer.
    //                             group.parallel_for_work_item([&](cl::sycl::h_item<1> h_item) {
    //                                 if (h_item.get_local().get_linear_id() == 0) {
    //                                     auto offset = h_item.get_global().get_linear_id();
    //                                     acc[offset + 0] = shmem[0];
    //                                     acc[offset + 1] = shmem[1];
    //                                 }
    //                             });
    //                         }
    //                     }
    //             );
    //         });
    //         auto acc = buf.get_access<cl::sycl::access::mode::read>();
    //         for(size_t i = 0; i < 4; ++i) {
    //             BOOST_REQUIRE(acc[i] == i);
    //         }
    //     }
    //
    //     // Functionality moved into a separate lambda function.
    //     {
    //         auto operate_on_shmem = [](cl::sycl::group<1> group, auto acc) {
    //             size_t shmem[2]; ///< Shared memory array.
    //
    //             // Populate shared memory with global linear IDs.
    //             group.parallel_for_work_item([&](cl::sycl::h_item<1> h_item) {
    //                 shmem[h_item.get_local().get_linear_id()] = h_item.get_global().get_linear_id();
    //             });
    //
    //             // Read from shared memory and write to buffer.
    //             group.parallel_for_work_item([&](cl::sycl::h_item<1> h_item) {
    //                 if (h_item.get_local().get_linear_id() == 0) {
    //                     auto offset = h_item.get_global().get_linear_id();
    //                     acc[offset + 0] = shmem[0];
    //                     acc[offset + 1] = shmem[1];
    //                 }
    //             });
    //         };
    //
    //         cl::sycl::buffer<size_t, 1> buf(4);
    //         queue.submit([&](cl::sycl::handler& cgh) {
    //             auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
    //             cgh.parallel_for_work_group<class shmem_two>(
    //                     cl::sycl::range<1>(2),
    //                     cl::sycl::range<1>(2),
    //                     [=](cl::sycl::group<1> group) {
    //                         operate_on_shmem(group, acc);
    //                     }
    //             );
    //         });
    //         auto acc = buf.get_access<cl::sycl::access::mode::read>();
    //         for(size_t i = 0; i < 4; ++i) {
    //             BOOST_REQUIRE(acc[i] == i);
    //         }
    //     }
    // }

    /**
     * @brief Forward declaration of a function.
     */
    void forward_declared2();

    /**
     * @brief Template forward declaration of a function.
     *
     * @tparam T The type parameter.
     */
    template<class T>
    void forward_declared1();

    /**
     * @brief Template forward declaration of a test kernel class.
     *
     * @tparam T The type parameter.
     */
    template<class T>
    class forward_declared_test_kernel;

    /**
     * @brief Test case for forward-declared functions.
     *
     * This test ensures that forward-declared functions are correctly handled
     * by the Clang plugin and emitted to device code.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(forward_declared_function)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    BOOST_AUTO_TEST_CASE(forward_declared_function) {
        cl::sycl::queue q;

        // The clang plugin must build the correct call graph and emit both
        // forward_declared1() and forward_declared2() to device code,
        // even though the call expression below refers to the forward declarations.
        q.submit([&](cl::sycl::handler& cgh){
            cgh.single_task<forward_declared_test_kernel<int>>([=](){
                forward_declared1<int>();
            });
        });
        q.wait_and_throw();
    }

    /**
     * @brief Template implementation that calls a forward-declared function.
     *
     * @tparam T The type parameter.
     */
    template <class T>
    void forward_declared1(){
        forward_declared2();
    }

    /**
     * @brief Definition of a forward-declared function.
     */
    void forward_declared2() {}

/**
 * @brief Advanced test case for optional lambda naming in SYCL kernels.
 *
 * This test verifies that unnamed lambdas are correctly handled by the SYCL
 * compiler, even when nested within other lambdas.
 *
 * @note This test is disabled if HIPSYCL_DISABLE_UNNAMED_LAMBDA_TESTS is defined.
 *
 * @example
 * @code
 * BOOST_AUTO_TEST_CASE(optional_lambda_naming)
 * {
 *     // Test implementation...
 * }
 * @endcode
 */
#ifndef HIPSYCL_DISABLE_UNNAMED_LAMBDA_TESTS
    BOOST_AUTO_TEST_CASE(optional_lambda_naming) {
        cl::sycl::queue q;
        auto lambda = [&]() {
            q.submit([&](cl::sycl::handler &cgh) {
                cgh.single_task([=]() { });
            });
            q.submit([&](cl::sycl::handler &cgh) {
                cgh.single_task([=]() { });
            });
            q.submit([&](cl::sycl::handler &cgh) {
                cgh.parallel_for(cl::sycl::range<1>{1024}, [=](cl::sycl::id<1> idx) { });
            });
            q.submit([&](cl::sycl::handler &cgh) {
                cgh.parallel_for(cl::sycl::range<1>{1024}, [=](cl::sycl::id<1> idx) { });
            });
        };
        lambda();
        q.wait_and_throw();
    }
#endif

    /**
     * @brief Variadic template structure for kernel naming.
     *
     * @tparam ... Types of the kernel parameters.
     */
    template <class ...>
    struct VariadicKernelTP {};

    /**
     * @brief Variadic non-type template structure for kernel naming.
     *
     * @tparam ... Non-type template parameters.
     */
    template <auto ...>
    struct VariadicKernelNTTP {};

    /**
     * @brief Test case for variadic kernel naming.
     *
     * This test verifies that kernels with variadic template parameters are correctly
     * named and handled by the SYCL compiler.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(variadic_kernel_name)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    BOOST_AUTO_TEST_CASE(variadic_kernel_name) {
        cl::sycl::queue queue;
        cl::sycl::buffer<size_t, 1> buf(1);

        {
            queue.submit([&](cl::sycl::handler& cgh) {
                auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
                cgh.parallel_for<VariadicKernelTP<int, bool, char>>(
                        cl::sycl::range<1>(1),
                        [=](cl::sycl::item<1>) {}
                );
            });
        }

        {
            queue.submit([&](cl::sycl::handler& cgh) {
                auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
                cgh.parallel_for<VariadicKernelNTTP<1, true, 'a'>>(
                        cl::sycl::range<1>(1),
                        [=](cl::sycl::item<1>) {}
                );
            });
        }
    }

    /**
     * @brief Function that calls another function.
     */
    void h() {}

    /**
     * @brief Function that calls another function.
     */
    void f() {
        h();
    }

    /**
     * @brief Function that does nothing.
     */
    void g() {}

    /**
     * @brief Test case for generic lambda outlining.
     *
     * This test checks that generic lambdas are correctly outlined and handled by
     * the SYCL compiler.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(generic_lambda_outlining)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    BOOST_AUTO_TEST_CASE(generic_lambda_outlining) {
        cl::sycl::queue q;
        q.parallel_for(cl::sycl::range{1024}, [=](auto item) {
            f();
            int x = 3;
            auto invoke = [&](auto value) {
                g();
            };
            invoke(x);
        });
    }

    /**
     * @brief Test case for nd_range kernels.
     *
     * This test verifies the correct handling of kernels launched with nd_range
     * in both one and two dimensions.
     *
     * @example
     * @code
     * BOOST_AUTO_TEST_CASE(nd_range)
     * {
     *     // Test implementation...
     * }
     * @endcode
     */
    BOOST_AUTO_TEST_CASE(nd_range) {
        cl::sycl::queue q;
        cl::sycl::buffer<size_t, 1> buf(1);

        {
            q.submit([&](cl::sycl::handler& cgh) {
                auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
                cgh.parallel_for(cl::sycl::nd_range<1>{{1}, {1}}, [=](cl::sycl::nd_item<1> item) {
                    acc[0] = 300 + item.get_global_linear_id();
                });
            });
            auto acc = buf.get_access<cl::sycl::access::mode::read>();
            BOOST_REQUIRE(acc[0] == 300);
        }

        {
            q.submit([&](cl::sycl::handler& cgh) {
                auto acc = buf.get_access<cl::sycl::access::mode::discard_write>(cgh);
                cgh.parallel_for(cl::sycl::nd_range<2>{{1,1}, {1,1}}, [=](cl::sycl::nd_item<2> item) {
                    acc[0] = 300 + item.get_global_linear_id();
                });
            });
            auto acc = buf.get_access<cl::sycl::access::mode::read>();
            BOOST_REQUIRE(acc[0] == 300);
        }
    }

BOOST_AUTO_TEST_SUITE_END()
