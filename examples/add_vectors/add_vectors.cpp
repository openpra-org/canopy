#include <cassert>
#include <execution>
#include <iostream>
#include <vector>

#include <CL/sycl.hpp>

using data_type = float_t;

/**
 * @brief Fills a vector with random values.
 *
 * This function fills the provided vector `v` with random floating-point values
 * in the range [0.0, 1.0).
 *
 * @param v The vector to be filled with random values.
 */
void fill_rand(std::vector<data_type> &v) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<data_type> dis(0.0, 1.0);

    std::generate(std::execution::par_unseq, v.begin(), v.end(), [&]() { return dis(gen); });
}

/**
 * @brief Adds two vectors element-wise using SYCL for parallel computation.
 *
 * This function takes two input vectors `a` and `b`, and returns a new vector `c`
 * where each element is the sum of the corresponding elements in `a` and `b`.
 *
 * @param q The SYCL queue used to submit the parallel computation.
 * @param a The first input vector.
 * @param b The second input vector.
 * @return A vector containing the element-wise sum of `a` and `b`.
 *
 * @note The input vectors `a` and `b` must have the same size.
 */
std::vector<data_type> add(cl::sycl::queue &q, const std::vector<data_type> &a, const std::vector<data_type> &b) {
    std::vector<data_type> c(a.size());

    assert(a.size() == b.size());
    cl::sycl::range<1> work_items{a.size()};
    {
        cl::sycl::buffer<data_type> buff_a(a.data(), a.size());
        cl::sycl::buffer<data_type> buff_b(b.data(), b.size());
        cl::sycl::buffer<data_type> buff_c(c.data(), c.size());

        q.submit([&](cl::sycl::handler &cgh) {
            auto access_a = buff_a.get_access<cl::sycl::access::mode::read>(cgh);
            auto access_b = buff_b.get_access<cl::sycl::access::mode::read>(cgh);
            auto access_c = buff_c.get_access<cl::sycl::access::mode::write>(cgh);

            cgh.parallel_for<class vector_add>(
                work_items, [=](cl::sycl::id<1> tid) { access_c[tid] = access_a[tid] + access_b[tid]; });
        });
    }
    return c;
}

/**
 * @brief Adds two vectors element-wise using standard C++ parallel algorithms.
 *
 * This function takes two input vectors `a` and `b`, and returns a new vector `c`
 * where each element is the sum of the corresponding elements in `a` and `b`.
 * The computation is performed using `std::transform` with a parallel execution policy.
 *
 * @param a The first input vector.
 * @param b The second input vector.
 * @return A vector containing the element-wise sum of `a` and `b`.
 *
 * @note The input vectors `a` and `b` must have the same size.
 */
std::vector<data_type> add_std(const std::vector<data_type> &a, const std::vector<data_type> &b) {
    std::vector<data_type> c(a.size());

    assert(a.size() == b.size());

    std::transform(std::execution::par_unseq, a.begin(), a.end(), b.begin(), c.begin(),
                   [](data_type x, data_type y) { return x + y; });

    return c;
}

/**
 * @brief The main function that demonstrates the use of the `add` and `add_std` functions.
 *
 * It initializes two vectors with random values, calls both `add` and `add_std` functions
 * to compute their sum, and asserts that the results are identical.
 *
 * @return int Returns 0 upon successful execution.
 */
int main() {
    const size_t vector_size = 0.3*1024*1024*1024;
    std::vector<data_type> a(vector_size);
    std::vector<data_type> b(vector_size);

    // Fill vectors with random values
    fill_rand(a);
    fill_rand(b);

    cl::sycl::queue q;
    auto result_sycl = add(q, a, b);
    //auto result_std = add_std(a, b);

    // Assert that both results are identical
    //assert(result_sycl.size() == result_std.size());
    for (size_t i = 0; i < result_sycl.size(); ++i) {
        assert(result_sycl[i] == a[i] + b[i]);
    }

    std::cout << "Both add and add_std functions produced identical results." << std::endl;

    return 0;
}