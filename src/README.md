
# $F_{\wedge}$: The AND-PLANE

## $F_{\wedge}$ Kernel
```cpp
  q.submit([&](sycl::handler& h) {
      // Accessors for inputs and product_terms
      auto inputs_acc = inputs_buf.get_access<sycl::access::mode::read>(h);
      auto prod_acc = product_terms_buf.get_access<sycl::access::mode::write>(h);
      
      h.parallel_for<class ANDPlaneKernel>(sycl::range<1>(num_products), [=](sycl::id<1> idx) {
          bool result = true;
          for (int i = 0; i < 2 * num_inputs; ++i) {
              if (AND_Plane[idx][i]) {
                  result &= inputs_acc[i];
              }
          }
          prod_acc[idx] = result;
      });
  });
```
```c++
  q.submit([&](sycl::handler& h) {
// Accessors for product_terms and outputs
auto prod_acc = product_terms_buf.get_access<sycl::access::mode::read>(h);
auto output_acc = outputs_buf.get_access<sycl::access::mode::write>(h);

h.parallel_for<class ORPlaneKernel>(sycl::range<1>(num_outputs), [=](sycl::id<1> idx) {
bool result = false;
for (int j = 0; j < num_products; ++j) {
if (OR_Plane[idx][j]) {
result |= prod_acc[j];
}
}
output_acc[idx] = result;
});
});
```

## [USM vs Buffers](https://www.intel.com/content/www/us/en/docs/oneapi/optimization-guide-gpu/2024-2/performance-impact-of-usm-and-buffers.html)
 
```c++

// for expression F = ab'c + a'b + bc' + a'bc' + aa'baacc'ab, with:
//
// s = 3 unique symbols (excluding negations)
// n = 2*s = 6 total symbols (including negations)
// m = 5 products
//
// using w=8-bit words to encode n=6 symbols, starting from the MSB, and,
// m = 5 words, we fill out a m=5 element vector,
// where each element encodes one product
// -------------------------------------------------
// |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
// -------------------------------------------------
// |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
// -------------------------------------------------
```