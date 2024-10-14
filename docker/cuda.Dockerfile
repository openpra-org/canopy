FROM nvidia/cuda:12.4.1-cudnn-devel-ubuntu22.04 AS baseimage

ENV DEBIAN_FRONTEND=noninteractive

## install apt-fast for faster downloads
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    apt update && \
    apt install -y --no-install-recommends software-properties-common && \
    add-apt-repository -y ppa:apt-fast/stable && \
    apt install -y --no-install-recommends apt-fast

ENV BUILD_PACKAGES="ca-certificates \
    curl \
    git \
    golang \
    gnupg \
    jq \
    lsb-release \
    nano \
    python3 \
    ssh \
    sudo \
    wget"

ENV SRC_BUILD_PACKAGES="build-essential \
    cmake \
    libboost-all-dev \
    libcln-dev \
    libgmp-dev \
    libginac-dev \
    automake \
    libglpk-dev \
    libhwloc-dev \
    libz3-dev \
    libxerces-c-dev \
    libeigen3-dev \
    doxygen \
    doxygen-doc \
    graphviz"

ENV DEBUGGER_PACKAGES="gdb \
    valgrind \
    linux-tools-generic \
    systemtap-sdt-dev \
    gdbserver \
    ccache \
    python3"

RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    apt-fast update && \
    apt-fast install -y --no-install-recommends $BUILD_PACKAGES $SRC_BUILD_PACKAGES $DEBUGGER_PACKAGES && \
    update-ca-certificates


## Install llvm 16
ENV LLVM_16_PACKAGES="libclang-16-dev \
    clang-tools-16 \
    libomp-16-dev \
    llvm-16-dev \
    lld-16"

RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 16 && \
    apt-fast update && \
    apt-fast install -y $LLVM_16_PACKAGES

## Install AdaptiveCpp
ENV ADAPTIVE_CPP_INSTALL_DIR="/usr/local"
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    git clone https://github.com/AdaptiveCpp/AdaptiveCpp && \
    cd AdaptiveCpp && mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=$ADAPTIVE_CPP_INSTALL_DIR .. && \
    make -j && \
    make install

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
