

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

```shell
/tmp/tmp.xQ8wveOZzU/cmake-build-release/src/bool/bool_et
M1 = 
[   0.84000   0.16000 ]
[   0.60000   0.40000 ]
[   0.66000   0.34000 ]

M2 = 
[   1.00000   0.00000   0.00000   0.00000   0.00000 ]
[   0.00000   0.20000   0.60000   0.10000   0.10000 ]

M3 = 
[   0.84000   0.03200   0.09600   0.01600   0.01600 ]
[   0.60000   0.08000   0.24000   0.04000   0.04000 ]
[   0.66000   0.06800   0.20400   0.03400   0.03400 ]

Entry State: Normal
  Probability of 'Alive, None, Not Cancelled': 0.84000
  Probability of 'Dead, Minor, Not Cancelled': 0.03200
  Probability of 'Dead, Moderate, Not Cancelled': 0.09600
  Probability of 'Dead, Major, Not Cancelled': 0.01600
  Probability of 'Dead, Major, Cancelled': 0.01600

Entry State: Sleepy
  Probability of 'Alive, None, Not Cancelled': 0.60000
  Probability of 'Dead, Minor, Not Cancelled': 0.08000
  Probability of 'Dead, Moderate, Not Cancelled': 0.24000
  Probability of 'Dead, Major, Not Cancelled': 0.04000
  Probability of 'Dead, Major, Cancelled': 0.04000

Entry State: Speeding
  Probability of 'Alive, None, Not Cancelled': 0.66000
  Probability of 'Dead, Minor, Not Cancelled': 0.06800
  Probability of 'Dead, Moderate, Not Cancelled': 0.20400
  Probability of 'Dead, Major, Not Cancelled': 0.03400
  Probability of 'Dead, Major, Cancelled': 0.03400


Process finished with exit code 0
```