cmake_minimum_required(VERSION 3.13)

project(vehicle_hal_grpc_server CXX)

# Override the settings in jsoncpp
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    CACHE PATH "Path of static libraries.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    CACHE PATH "Path of dynamic libraries.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Path of executables.")
mark_as_advanced(CMAKE_RUNTIME_OUTPUT_DIRECTORY CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_ARCHIVE_OUTPUT_DIRECTORY)

set(_trout_CXX_FLAGS -Wall -Werror -std=c++17)

set(_trout_GRPCXX_LIBRARY_NAME "grpc++")

include(cmake/fmtlib.cmake)
include(cmake/android.cmake)
include(cmake/default_native_vhal.cmake)
include(cmake/grpc.cmake)
include(cmake/jsoncpp.cmake)
include(cmake/protobuf.cmake)

get_filename_component(trout_VHAL_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
set(trout_PREBUILT_DIR ${trout_VHAL_SRC_ROOT}/agl_build/prebuilt)
set(trout_GENS_DIR ${trout_VHAL_SRC_ROOT}/agl_build/build/gens)

if (NOT trout_NATIVE_VHAL_ROOT_DIR)
    set(trout_NATIVE_VHAL_ROOT_DIR ${trout_VHAL_SRC_ROOT}/agl_build/third_party/default_native_vhal)
endif()

# ========== grpc proto prebuilts =================

find_program(trout_PROTOC aprotoc REQUIRED)
find_program(trout_GRPC_CPP_PLUGIN protoc-gen-grpc-cpp-plugin REQUIRED)

if(NOT _trout_PROTOBUF_WELLKNOWN_INCLUDE_DIR)
  message(FATAL_ERROR "_trout_PROTOBUF_WELLKNOWN_INCLUDE_DIR not set")
endif()


function(trout_generate_grpc_cpp_from_proto)
  if(NOT ARGN)
    message(SEND_ERROR "Error: trout_generate_grpc_cpp_from_proto() called without any proto files")
    return()
  endif()

  set(_protobuf_include_path -I . -I ${_trout_PROTOBUF_WELLKNOWN_INCLUDE_DIR}
      -I ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/proto)

  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(ABS_DIR ${ABS_FIL} DIRECTORY)
    get_filename_component(FIL_NAME ${FIL} NAME)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    add_custom_command(
      OUTPUT "${trout_GENS_DIR}/${FIL_WE}.grpc.pb.cc"
             "${trout_GENS_DIR}/${FIL_WE}.grpc.pb.h"
             "${trout_GENS_DIR}/${FIL_WE}.pb.cc"
             "${trout_GENS_DIR}/${FIL_WE}.pb.h"
             COMMAND ${trout_PROTOC}
      ARGS --grpc_out=generate_mock_code=true:${trout_GENS_DIR}
           --cpp_out=${trout_GENS_DIR}
           --plugin=protoc-gen-grpc=${trout_GRPC_CPP_PLUGIN}
           ${_protobuf_include_path}
           ${FIL_NAME}
      DEPENDS ${ABS_FIL} ${trout_PROTOC} ${trout_GRPC_CPP_PLUGIN}
      WORKING_DIRECTORY ${ABS_DIR}
      COMMENT "Running gRPC C++ protocol buffer compiler on ${FIL}"
      VERBATIM)

      set_source_files_properties("${trout_GENS_DIR}/${FIL_WE}.grpc.pb.cc" "${trout_GENS_DIR}/${FIL_WE}.grpc.pb.h" "${trout_GENS_DIR}/${FIL_WE}.pb.cc" "${trout_GENS_DIR}/${FIL_WE}.pb.h" PROPERTIES GENERATED TRUE)
  endforeach()
endfunction()


trout_generate_grpc_cpp_from_proto(
    ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/proto/VehicleHalProto.proto
    ${trout_VHAL_SRC_ROOT}/proto/VehicleServer.proto)

# ========== Protobuf generated code ==================

add_library(vehicle_hal_protobuf_gen
    ${trout_GENS_DIR}/VehicleHalProto.pb.cc
    ${trout_GENS_DIR}/VehicleHalProto.grpc.pb.cc
    ${trout_GENS_DIR}/VehicleServer.pb.cc
    ${trout_GENS_DIR}/VehicleServer.grpc.pb.cc
)

target_link_libraries(vehicle_hal_protobuf_gen
    ${trout_GRPCXX_LIBRARIES}
)

target_compile_options(vehicle_hal_protobuf_gen PRIVATE ${_trout_CXX_FLAGS} -Wno-array-bounds)

# ========== vehicle_hal_grpc_server ==============

add_executable(vehicle_hal_grpc_server
    ${trout_VHAL_SRC_ROOT}/VirtualizationGrpcServer.cpp
    ${trout_VHAL_SRC_ROOT}/GarageModeServerSideHandler.cpp
    ${trout_VHAL_SRC_ROOT}/GrpcVehicleServer.cpp
    ${trout_VHAL_SRC_ROOT}/PowerStateListener.cpp
    ${trout_VHAL_SRC_ROOT}/Utils.cpp

    ${trout_NATIVE_VHAL_ROOT_DIR}/common/src/Obd2SensorStore.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/common/src/VehicleObjectPool.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/common/src/VehicleUtils.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/VehicleHalServer.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/EmulatedUserHal.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/GeneratorHub.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/JsonFakeValueGenerator.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/LinearFakeValueGenerator.cpp
    ${trout_NATIVE_VHAL_ROOT_DIR}/impl/vhal_v2_0/ProtoMessageConverter.cpp
)

target_include_directories(vehicle_hal_grpc_server
    PRIVATE ${trout_ANDROID_INCLUDE_DIRS}
    PRIVATE ${trout_JSONCPP_INCLUDE_DIRS}
    PRIVATE ${trout_NAVTIVE_VHAL_COMMON_INCLUDE_DIRS}
    PRIVATE ${trout_NAVTIVE_VHAL_IMPL_INCLUDE_DIRS}
    PRIVATE ${trout_PREBUILT_DIR}/include
    PRIVATE ${trout_GENS_DIR}
)

target_link_libraries(vehicle_hal_grpc_server
    vehicle_hal_protobuf_gen
    ${trout_ANDROID_LIBRARIES}
    ${trout_JSONCPP_LIBRARIES}
)

target_compile_options(vehicle_hal_grpc_server PRIVATE ${_trout_CXX_FLAGS})
