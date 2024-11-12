

## Sample Output
```shell
/tmp/tmp.xlkWqx0nkl/cmake-build-release/src/bool/bool
:::::::::::::::::::::::::::: PROFILER SUMMARY [ns] ::::::::::::::::::::::::::::::
runs: [1] : generate random number vector, num_samples=1e7, float32
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
[min,  max] : [1.37578679e+10, 1.37578679e+10]
[avg,  std] : [1.37578679e+10, 0.00000000e+00]
[5th, 95th] : [1.37578679e+10, 1.37578679e+10]
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
Device Information:
General Information:
  Device Name: NVIDIA GeForce GTX 1660 SUPER
  Vendor Name: NVIDIA
  Driver Version: 12040
  Profile: FULL_PROFILE
  Version: sm_75
  OpenCL C Version: 1.2 HIPSYCL
  Device Type: GPU
  Vendor ID: 4318
  Platform: CUDA
  Is Available: true
  Is Compiler Available: true
  Is Linker Available: true
  Reference Count: 1
  Available Extensions:

Compute Unit Information:
  Max Compute Units: 22
  Max Work Item Dimensions: 3
  Max Work Item Sizes: 64 1024 1024 
  Max-Num Sub-Groups: 32
  Sub-group Sizes: 32 
  Sub-group Independent Forward Progress: true
  Max Work Group Size: 1024
  Preferred Vector Widths:
    char: 4
    short: 2
    int: 1
    long: 1
    float: 1
    double: 1
    half: 2
  Native Vector Widths:
    char: 4
    short: 2
    int: 1
    long: 1
    float: 1
    double: 1
    half: 2
  Max Clock Frequency: 1785 MHz
  Address Bits: 64
  Max Samplers: 0
  Max Parameter Size: 18446744073709551615
  Mem Base Address Align: 8 bits
  Profiling Timer Resolution: 1
  Endian Little: true

Memory Information:
  Global Memory Size: 5929 MB
  Max Memory Allocation Size: 5929 MB
  Local Memory Size: 48 KB
  Local Memory Type: local
  Error Correction Support: false
  Host Unified Memory: false
  Global Memory Cache Type: read_write
  Global Memory Cache Line Size: 128
  Global Memory Cache Size: 1536 KB
  Max Constant Buffer Size: 64 KB
  Max Constant Args: 4294967295
  Printf Buffer Size: 18446744073709551615
  Preferred Interop User Sync: true

Image Support Information:
  Image Support: false

Floating Point Configurations:
  Half Precision FP Configurations:
    denorm
    inf_nan
    round_to_nearest
    round_to_zero
    round_to_inf
    fma
    correctly_rounded_divide_sqrt
  Single Precision FP Configurations:
    denorm
    inf_nan
    round_to_nearest
    round_to_zero
    round_to_inf
    fma
    correctly_rounded_divide_sqrt
  Double Precision FP Configurations:
    denorm
    inf_nan
    round_to_nearest
    round_to_zero
    round_to_inf
    fma
    correctly_rounded_divide_sqrt

Execution Capabilities:
  exec_kernel
  Queue Profiling: true
  Built-in Kernels:

Partition Information:
  Partition Max Sub-devices: 0
  Partition Properties:
  Partition Affinity Domains:
    not_applicable
  Partition Type Property: no_partition
  Partition Type Affinity Domain: not_applicable

Aspects Supported:
  gpu
  accelerator
  fp64
  atomic64
  queue_profiling
  usm_device_allocations
  usm_host_allocations
  usm_shared_allocations

Other Information:
  [Error retrieving other information: Device is not a subdevice]

sub_group_size: 32
max_work_item_size: 1024
max_work_group_size: 1024
num_work_groups: 1078
total_work_items: 1103872
samples_per_work_item: 906
F_per_group: 928

[AdaptiveCpp Warning] kernel_cache: This application run has resulted in new binaries being JIT-compiled. This indicates that the runtime optimization process has not yet reached peak performance. You may want to run the application again until this warning no longer appears to achieve optimal performance.
P(a): 1.000000047497451e-03
P(b): 9.999999747378752e-05
P(c): 9.999999747378752e-06
total simulations         [n]: 1000000000
true evaluations          [T]: 101600
false evaluations         [F]: 999898400
known expected value P(f) [μ]: 1.000099873635918e-04
estimated mean            [m]: 1.016000023810193e-04
estimated variance        [v]: 1.015896757992560e-13
std. error √[m•(1-m)/n] = [s]: 3.187313666330738e-07
absolute error  |μ – m| = [ε]: 1.590015017427504e-06
percentage error (ε/μ)% = [%]: 1.589856266975403e+00
95% CI: 1.009753032121807e-04, 1.022247015498579e-04
99% CI: 1.007790051517077e-04, 1.024209996103309e-04

:::::::::::::::::::::::::::: PROFILER SUMMARY [ns] ::::::::::::::::::::::::::::::
runs: [5] : Optimized evaluation of F with blocking
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
[min,  max] : [3.16697395e+09, 3.42759327e+09]
[avg,  std] : [3.22126807e+09, 1.03184582e+08]
[5th, 95th] : [3.16697395e+09, 3.42759327e+09]
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