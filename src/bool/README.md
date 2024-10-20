

## Sample Output
```shell
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
[min,  max] : [1.74343243e+10, 1.74439892e+10]
[avg,  std] : [1.74391568e+10, 4.83245550e+06]
[5th, 95th] : [1.74343243e+10, 1.74439892e+10]
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Process finished with exit code 0
```