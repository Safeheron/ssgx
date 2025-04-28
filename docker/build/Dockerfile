FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive

# We assume this docker file is invoked with root at the top of ssgx repo.
RUN apt-get update && apt-get install  -y \
    dkms \
    wget \
    autoconf \
    build-essential \
    libtool \
    pkg-config \
    automake \
    git \
    vim \
    openssl \
    libssl-dev \
    perl \
    tar \
    pkg-config \
    unzip \
    ninja-build

WORKDIR /src

RUN wget https://github.com/Kitware/CMake/releases/download/v3.22.5/cmake-3.22.5-linux-x86_64.sh \
    && sudo sh cmake-3.22.5-linux-x86_64.sh --skip-license --prefix=/usr \
    && rm cmake-3.22.5-linux-x86_64.sh

RUN wget https://download.01.org/intel-sgx/sgx-linux/2.23/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.23.100.2.bin \
    && ((echo 'no/'; echo '/opt/intel/') | sudo bash sgx_linux_x64_sdk_2.23.100.2.bin) \
    && rm sgx_linux_x64_sdk_2.23.100.2.bin

RUN git clone --recurse-submodules -b sgx_2.23 https://github.com/intel/linux-sgx.git \
    && cd linux-sgx/external/protobuf \
    && make -j$(nproc) \
    && mkdir /opt/intel/sgxsdk/include/tprotobuf/google/protobuf/util \
    && cp protobuf_code/src/google/protobuf/util/*.h /opt/intel/sgxsdk/include/tprotobuf/google/protobuf/util/ \
    && cd /src && rm -r linux-sgx

RUN wget https://download.01.org/intel-sgx/sgx-linux/2.23/distro/ubuntu20.04-server/sgx_debian_local_repo.tgz \
    && tar -zxf sgx_debian_local_repo.tgz \
    && sudo mv sgx_debian_local_repo /srv/ \
    && echo "deb [trusted=yes arch=amd64] file:/srv/sgx_debian_local_repo focal main" > /etc/apt/sources.list.d/sgx_debian_local_repo.list \
    && apt update \
    && sudo apt-get -y install libsgx-enclave-common-dev libsgx-dcap-ql-dev libsgx-dcap-default-qpl-dev libsgx-dcap-quote-verify-dev libsgx-quote-ex-dev \
    && rm sgx_debian_local_repo.tgz

RUN wget https://download.01.org/intel-sgx/sgx-linux/2.23/as.ld.objdump.r4.tar.gz \
    && tar -zxf as.ld.objdump.r4.tar.gz \
    && sudo cp external/toolset/ubuntu20.04/* /usr/local/bin \
    && which ar as ld objcopy objdump ranlib \
    && rm -r as.ld.objdump.r4.tar.gz external

RUN wget https://github.com/intel/intel-sgx-ssl/archive/refs/tags/lin_2.21_1.1.1u.tar.gz \
    && tar -zxvf lin_2.21_1.1.1u.tar.gz \
    && rm lin_2.21_1.1.1u.tar.gz \
    && cd intel-sgx-ssl-lin_2.21_1.1.1u/openssl_source \
    && wget https://www.openssl.org/source/openssl-1.1.1u.tar.gz \
    && cd ../Linux \
    && make -j$(nproc) \
    && sudo make install \
    && cd ../.. && rm -r intel-sgx-ssl-lin_2.21_1.1.1u

COPY ./external /src/external
COPY ./build_install_external.sh /src/build_install_external.sh

RUN sudo bash build_install_external.sh \
    && cd /src && rm -r build_install_external.sh external

