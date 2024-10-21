

## Sample Output
```shell
/Users/arjun/Projects/OpenPRA/canopy/cmake-build-release/src/bool/bool
:::::::::::::::::::::::::::: PROFILER SUMMARY [ns] ::::::::::::::::::::::::::::::
runs: [1] : generate random number vector, num_samples=1e7, float32
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
[min,  max] : [1.62793640e+09, 1.62793640e+09]
[avg,  std] : [1.62793640e+09, 0.00000000e+00]
[5th, 95th] : [1.62793640e+09, 1.62793640e+09]
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
[AdaptiveCpp Warning] from /build/AdaptiveCpp/src/runtime/ocl/ocl_hardware_manager.cpp:628 @ ocl_hardware_manager(): ocl_hardware_manager: Shared context construction failed. Will attempt to fall back to individual context per device, but this may prevent data transfers between devices from working. (error code = CL:-30)
[AdaptiveCpp Warning] OpenCL device NVIDIA GeForce GTX 1660 SUPER does not have a valid USM provider. Memory allocations are not possible on that device.
[AdaptiveCpp Warning] kernel_cache: This application run has resulted in new binaries being JIT-compiled. This indicates that the runtime optimization process has not yet reached peak performance. You may want to run the application again until this warning no longer appears to achieve optimal performance.
P(a): 1.000000047497451e-03
P(b): 9.999999747378752e-05
P(c): 9.999999747378752e-06
total simulations         [n]: 10000000
true evaluations          [T]: 1011
false evaluations         [F]: 9998989
known expected value P(f) [μ]: 1.000099873635918e-04
estimated mean            [m]: 1.010999985737726e-04
estimated variance        [v]: 1.010897789088805e-11
std. error √[m•(1-m)/n] = [s]: 3.179461828040075e-06
absolute error  |μ – m| = [ε]: 1.090011210180819e-06
percentage error (ε/μ)% = [%]: 1.089902400970459e+00
95% CI: 9.486836643191054e-05, 1.073316307156347e-04
99% CI: 9.291024616686627e-05, 1.092897509806789e-04

:::::::::::::::::::::::::::: PROFILER SUMMARY [ns] ::::::::::::::::::::::::::::::
runs: [2] : F=ab'c+a'b+bc', x=3, term<width>=uint_fast8_t, products=1e6, samples=1e7
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
[min,  max] : [1.70865402e+10, 1.71368906e+10]
[avg,  std] : [1.71117154e+10, 2.51751825e+07]
[5th, 95th] : [1.70865402e+10, 1.71368906e+10]
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Process finished with exit code 0

```