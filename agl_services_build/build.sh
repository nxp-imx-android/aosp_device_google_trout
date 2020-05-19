#!/bin/bash

build_common() {
    local current_dir=`pwd`
    local bash_src_dir=$(realpath $(dirname ${BASH_SOURCE[0]}))
    local build_type=$1
    local cmake_options=""
    shift
    if [[ ${build_type} == "agl" ]]; then
        cmake_options="${cmake_options} -DCMAKE_TOOLCHAIN_FILE=${bash_src_dir}/toolchain/agl_toolchain.cmake"
    fi

    mkdir -p ${TROUT_SRC_ROOT:-${bash_src_dir}}/out/${build_type}_build && cd $_
    cmake -G Ninja ${cmake_options} ../..
    ninja $@
    cd ${current_dir}
}

build_host_tools() {
    build_common host $@
}

build_agl_service() {
    build_common agl $@
}

if [[ ! $(which aprotoc) && ! $(which protoc-gen-grpc-cpp-plugin) ]]; then
    build_host_tools protoc grpc_cpp_plugin
fi

build_agl_service vehicle_hal_grpc_server dumpstate_grpc_server watchdog_test_service
