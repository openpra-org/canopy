# Release Build
```bash
coder@81e90788d46d:/tmp/tmp.xlkWqx0nkl/cmake-build-release/src/bool$ nvprof ./bool 
:::::::::::::::::::::::::::: PROFILER SUMMARY [ns] ::::::::::::::::::::::::::::::
runs: [1] : generate random number vector, num_samples=1e7, float32
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
[min,  max] : [2.57469908e+10, 2.57469908e+10]
[avg,  std] : [2.57469908e+10, 0.00000000e+00]
[5th, 95th] : [2.57469908e+10, 2.57469908e+10]
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
==2012== NVPROF is profiling process 2012, command: ./bool
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
P(a): 1.000000047497451e-03
P(b): 9.999999747378752e-05
P(c): 9.999999747378752e-06
total simulations         [n]: 1000000000
true evaluations          [T]: 100247
false evaluations         [F]: 999899753
known expected value P(f) [μ]: 1.000099873635918e-04
estimated mean            [m]: 1.002470016828738e-04
estimated variance        [v]: 1.002369506299637e-13
std. error √[m•(1-m)/n] = [s]: 3.166021826928045e-07
absolute error  |μ – m| = [ε]: 2.370143192820251e-07
percentage error (ε/μ)% = [%]: 2.369906455278397e-01
95% CI: 9.962647163774818e-05, 1.008675317279994e-04
99% CI: 9.943149052560329e-05, 1.010625128401443e-04

:::::::::::::::::::::::::::: PROFILER SUMMARY [ns] ::::::::::::::::::::::::::::::
runs: [5] : Optimized evaluation of F with blocking
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
[min,  max] : [3.20136828e+09, 3.36728048e+09]
[avg,  std] : [3.24430858e+09, 6.18431128e+07]
[5th, 95th] : [3.20136828e+09, 3.36728048e+09]
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
==2012== Profiling application: ./bool
==2012== Profiling result:
            Type  Time(%)      Time     Calls       Avg       Min       Max  Name
 GPU activities:   99.34%  16.0582s         5  3.21164s  3.20117s  3.21989s  _Z18__acpp_sscp_kernelIN7hipsycl4glue15__sscp_dispatch20ndrange_parallel_forIZZZ4mainENK3$_1clEvENKUlRNS0_4sycl7handlerEE_clES7_EUlNS5_7nd_itemILi1EEEE_Li1EEEEvRKT_
                    0.66%  106.46ms         7  15.209ms  1.1830us  106.37ms  [CUDA memcpy HtoD]
                    0.00%  1.7920us         1  1.7920us  1.7920us  1.7920us  [CUDA memcpy DtoH]
      API calls:   98.87%  16.0581s        10  1.60581s  2.8270us  3.21989s  cudaEventSynchronize
                    0.66%  106.58ms         8  13.323ms  4.9270us  106.38ms  cudaMemcpyAsync
                    0.34%  55.638ms         3  18.546ms  6.5770us  55.517ms  cudaMalloc
                    0.12%  20.008ms         1  20.008ms  20.008ms  20.008ms  cuDevicePrimaryCtxRelease
                    0.00%  742.81us         4  185.70us  1.6110us  370.43us  cudaFree
                    0.00%  111.60us       114     978ns     102ns  44.147us  cuDeviceGetAttribute
                    0.00%  85.056us         1  85.056us  85.056us  85.056us  cuModuleLoadDataEx
                    0.00%  80.071us         1  80.071us  80.071us  80.071us     �
                    0.00%  69.387us         5  13.877us  13.608us  14.351us  cuLaunchKernel
                    0.00%  45.917us        17  2.7010us  1.1370us  7.8730us  cudaEventRecord
                    0.00%  21.425us         4  5.3560us  1.5030us  16.212us  cudaStreamCreateWithFlags
                    0.00%  17.898us         4  4.4740us  1.8690us  12.036us  cudaStreamDestroy
                    0.00%  16.042us         1  16.042us  16.042us  16.042us  cuModuleUnload
                    0.00%  10.883us         1  10.883us  10.883us  10.883us  cuDeviceGetName
                    0.00%  10.103us         6  1.6830us     489ns  2.2070us  cudaStreamWaitEvent
                    0.00%  9.2730us         8  1.1590us     557ns  2.4110us  cudaEventCreate
                    0.00%  8.0960us         2  4.0480us  1.5950us  6.5010us  cudaGetDevice
                    0.00%  6.1450us         1  6.1450us  6.1450us  6.1450us  cuDeviceGetPCIBusId
                    0.00%  5.9300us         5  1.1860us     669ns  1.4760us  cuModuleGetFunction
                    0.00%  5.8330us         3  1.9440us     716ns  3.0480us  cudaPointerGetAttributes
                    0.00%  4.0700us         8     508ns     353ns     933ns  cudaEventDestroy
                    0.00%  3.1940us         1  3.1940us  3.1940us  3.1940us  cudaGetDeviceCount
                    0.00%  1.5270us         1  1.5270us  1.5270us  1.5270us  cudaDriverGetVersion
                    0.00%  1.0430us         3     347ns     161ns     673ns  cuDeviceGetCount
                    0.00%     634ns         2     317ns     124ns     510ns  cuDeviceGet
                    0.00%     414ns         1     414ns     414ns     414ns  cuDeviceTotalMem
                    0.00%     276ns         1     276ns     276ns     276ns  cuDeviceGetUuid
                    0.00%     231ns         1     231ns     231ns     231ns

```