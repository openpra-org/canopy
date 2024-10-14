#include <cassert>
#include <iostream>
#include <vector>

#include <CL/sycl.hpp>

using data_type = float;

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
 * @brief The main function that demonstrates the use of the `add` function.
 *
 * It initializes two vectors, calls the `add` function to compute their sum,
 * and prints the result to the standard output.
 *
 * @return int Returns 0 upon successful execution.
 */
int main() {
    cl::sycl::queue q;
    std::vector<data_type> a = {1.f, 2.f, 3.f, 4.f, 5.f};
    std::vector<data_type> b = {-1.f, 2.f, -3.f, 4.f, -5.f};
    auto result = add(q, a, b);

    std::cout << "Result: " << std::endl;
    for (const auto x : result)
        std::cout << x << std::endl;

    return 0;
}
