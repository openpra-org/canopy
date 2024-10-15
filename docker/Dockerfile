FROM nvidia/cuda:12.4.1-cudnn-devel-ubuntu22.04 AS baseimage

ENV CODENAME="jammy"
ENV DEBIAN_FRONTEND=noninteractive

## install apt-fast for faster downloads
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    apt update && \
    apt install -y --no-install-recommends software-properties-common && \
    add-apt-repository -y ppa:apt-fast/stable && \
    apt install -y --no-install-recommends apt-fast && \
    apt-fast upgrade -y --no-install-recommends

ENV BUILD_PACKAGES="ca-certificates \
    curl \
    file \
    git \
    golang \
    gnupg \
    jq \
    lsb-release \
    nano \
    python3 \
    rsync \
    ssh \
    sudo \
    unzip \
    wget"

ENV SRC_BUILD_PACKAGES="automake \
    cmake \
    doxygen \
    doxygen-doc \
    graphviz \
    libboost-all-dev \
    libcln-dev \
    libcurl4-openssl-dev \
    libedit-dev \
    libeigen3-dev \
    libgmp-dev \
    libginac-dev \
    libglpk-dev \
    libhwloc-dev \
    libnuma-dev \
    libxerces-c-dev \
    libz3-dev \
    libzstd-dev \
    spirv-cross \
    spirv-headers \
    spirv-tools"

ENV DEBUGGER_PACKAGES="gdb \
    valgrind \
    linux-tools-generic \
    systemtap-sdt-dev \
    gdbserver \
    ccache \
    python3"

ENV UTIL_PACKAGES="nvtop intel-gpu-tools"

RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    apt-fast update && \
    apt-fast install -y --no-install-recommends $BUILD_PACKAGES $SRC_BUILD_PACKAGES $DEBUGGER_PACKAGES $UTIL_PACKAGES && \
    update-ca-certificates

## Install llvm/clang
ENV CLANG_VERSION="18"
ENV LLVM_PACKAGES="libclang-$CLANG_VERSION-dev \
    clang-tools-$CLANG_VERSION \
    libomp-$CLANG_VERSION-dev \
    llvm-$CLANG_VERSION-dev \
    lld-$CLANG_VERSION"

RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh $CLANG_VERSION && \
    apt-fast update && \
    apt-fast install -y --no-install-recommends $LLVM_PACKAGES

## Install Intel OneAPI OpenCL
ENV INTEL_ONEAPI_OPENCL_PACKAGES="intel-oneapi-runtime-opencl-2024 intel-oneapi-runtime-compilers-2024 ocl-icd-libopencl1 ocl-icd-opencl-dev intel-oneapi-runtime-dpcpp* intel-oneapi-runtime-opencl*"
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB | gpg --dearmor | tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null && \
    echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list && \
    apt-fast update && \
    apt-fast install -y $INTEL_ONEAPI_OPENCL_PACKAGES
#    apt-fast autoremove -y

## Install Intel Compute Runtime
ARG INTEL_LZ_DEBS=intel-lz-deb.list
WORKDIR /intel-lz
COPY ./apt-sources/$INTEL_LZ_DEBS .
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    aria2c -j8 -i $INTEL_LZ_DEBS && \
    sha256sum -c ww35.sum && \
    dpkg -i *.deb

WORKDIR /
## Compile & Install Intel OneAPI Level Zero for Intel GPU support
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    git clone https://github.com/oneapi-src/level-zero.git && \
    cd level-zero && mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -j --target package && \
    cmake --build . -j --target install

## Install Nvidia GPUs as OpenCL target as well
WORKDIR /
ENV NVIDIA_OPENCL_PACKAGES="ocl-icd-libopencl1 opencl-headers clinfo"
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    apt-fast update && \
    apt-fast install -y --no-install-recommends $NVIDIA_OPENCL_PACKAGES && \
    mkdir -p /etc/OpenCL/vendors && \
    echo "libnvidia-opencl.so.1" > /etc/OpenCL/vendors/nvidia.icd

## Install AMD GPU Driver, OpenCL & ROCm backends

ARG URL_AMD_DRIVER="https://repo.radeon.com/amdgpu-install/6.2.3/ubuntu/jammy/amdgpu-install_6.2.60203-1_all.deb"
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    curl $URL_AMD_DRIVER -o amdgpu.deb && \
    dpkg -i amdgpu.deb && \
    amdgpu-install --usecase=opencl,graphics --opencl=rocr --vulkan=pro,amdvlk --no-dkms --accept-eula -y && \
    mkdir -p /etc/OpenCL/vendors && \
    echo "libamdocl64.so" > /etc/OpenCL/vendors/amdocl64.icd && \
    ln -s /usr/lib/x86_64-linux-gnu/libOpenCL.so.1 /usr/lib/libOpenCL.so && \
    apt-fast update && \
    apt-fast install -y --no-install-recommends rocm-dev

## Install AdaptiveCpp
ENV ADAPTIVE_CPP_INSTALL_DIR="/usr/local"
ENV CMAKE_C_COMPILER="/usr/bin/clang-$CLANG_VERSION"
ENV CMAKE_CXX_COMPILER="/usr/bin/clang++-$CLANG_VERSION"
ENV CLANG_EXECUTABLE_PATH="/usr/bin/clang++-$CLANG_VERSION"
ENV LLVM_DIR="/usr/lib/llvm-$CLANG_VERSION/cmake"
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    git clone https://github.com/AdaptiveCpp/AdaptiveCpp && \
    cd AdaptiveCpp && mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=$ADAPTIVE_CPP_INSTALL_DIR \
          -DCMAKE_C_COMPILER=$CMAKE_C_COMPILER \
          -DCMAKE_CXX_COMPILER=$CMAKE_CXX_COMPILER \
          -DCLANG_EXECUTABLE_PATH=$CLANG_EXECUTABLE_PATH \
          -DLLVM_DIR=$LLVM_DIR \
          -DWITH_CUDA_BACKEND=ON \
          -DWITH_OPENCL_BACKEND=ON \
          -DWITH_ROCM_BACKEND=ON \
          -DWITH_LEVEL_ZERO_BACKEND=ON .. && \
    make -j && \
    make install

## setup a build/dev environment
FROM baseimage AS devimage
ARG USER=coder
ARG UID=1000
ARG GID=1000
RUN if getent passwd ${UID}; then userdel -f $(getent passwd ${UID} | cut -d: -f1); fi && \
    if getent group ${GID}; then groupdel $(getent group ${GID} | cut -d: -f1); fi && \
    groupadd -g ${GID} ${USER} && \
    useradd --uid ${UID} --gid ${GID} --groups sudo --create-home --shell /bin/bash ${USER} && \
    echo "${USER} ALL=(ALL) NOPASSWD:ALL" >/etc/sudoers.d/${USER} && \
    chmod 0440 /etc/sudoers.d/${USER} && \
    mkdir -p /home/${USER}/projects && \
    chown -R ${USER}:${USER} /home/${USER}

USER ${USER}
WORKDIR /home/${USER}/projects/canopy

#FROM devimage AS buildimage
#COPY . .

#WORKDIR /home/${USER}/projects/canopy/build
#RUN cmake .. && \
#    make -j 
