# canopy
platform-independent sparse matrix library for quantifying large PRA models

## Setup & Install
This project uses git submodules. Begin by cloning recursively, or initializing and updating the repo.
```shell
git clone --recursive git@github.com:openpra-org/canopy.git
```
```shell
git submodule update --init --recursive
```

## Development

### Develop Using Docker/Linux
This is the preferred method. You can directly use the [Dockerfile](Dockerfile) to set up your build environment.
If you're building directly in Linux, please follow the steps in the Dockerfile for your build environment. Development
in other environments should be possible but has not been documented yet. Please open a pull request if you'd like to contribute to this effort.

#### docker build

```bash
docker context create ssh-box --docker "host=ssh://user@my-box"
```
```bash
docker build -t canopy:ssh-debugger --target=ssh-debugger .
```

#### docker run
In daemon mode:
```bash
docker run -d \
  --name=canopy-dev \
  -p 2222:2222 \
  --device=/dev/dri \
  --device=/dev/kfd \
  --cap-add=ALL \
  --security-opt seccomp=unconfined \
  --group-add video \
  canopy:ssh-debugger
  canopy:ssh-debugger
```

```bash
oder
```

### Develop on macOS
Since macOS does not support OpenCL, CUDA, HIP, Level Zero, ...ugh anything but Metal and OpenMP, support is currently
limited. It should still be possible to develop for testing purposes. Start by installing brew, and then AdaptiveCpp.
Please open a PR if you make any progress here...

```zsh
brew install seanfarley/adaptivecpp/adaptivecpp
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
