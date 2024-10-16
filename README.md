# canopy
platform-independent sparse matrix library for quantifying large PRA models


## Development

### Docker
Use/follow the [Dockerfile](docker/generic.Dockerfile) to set up your build environment.
#### docker build
```bash
docker build \
    -t canopy-dev:latest \
    -f docker/generic.Dockerfile .
```
### docker run
```bash
docker run --rm -it \
    --device=/dev/kfd \
    --device=/dev/dri:/dev/dri \
    --cap-add=ALL \ # unsafe, only needed for optional intel_gpu_top
    --group-add video \
    /bin/bash
```

### Tested Versions
- CUDA: 12.4.1
- LLVM/Clang: 18.1.8
- AMD ROCm: 6.2.3
- OpenCL: 
  - Intel OneAPI OpenCL 3.0
  - OpenCL 2.1 AMD-APP (3625.0)
  - OpenCL 3.0 CUDA

### AdaptiveCpp Detected Backends
Note: No HIP device since this host does not have a AMD GPU.
```bash
coder@6954020af52d:~/projects/canopy$ acpp-info -l
=================Backend information===================
Loaded backend 0: CUDA
  Found device: NVIDIA GeForce GTX 1660 SUPER
Loaded backend 1: HIP
  (no devices found)
Loaded backend 2: Level Zero
  Found device: Intel(R) UHD Graphics 630
Loaded backend 3: OpenCL
  Found device: Intel(R) UHD Graphics 630
  Found device: Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz
Loaded backend 4: OpenMP
  Found device: AdaptiveCpp OpenMP host device
```
### OpenCL Backend support
```bash
coder@6954020af52d:~/projects/canopy$ clinfo -l
Platform #0: AMD Accelerated Parallel Processing
Platform #1: Intel(R) OpenCL Graphics
 `-- Device #0: Intel(R) UHD Graphics 630
Platform #2: Intel(R) OpenCL
 `-- Device #0: Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz
Platform #3: Intel(R) FPGA Emulation Platform for OpenCL(TM)
 `-- Device #0: Intel(R) FPGA Emulation Device
Platform #4: NVIDIA CUDA
 `-- Device #0: NVIDIA GeForce GTX 1660 SUPER
```
