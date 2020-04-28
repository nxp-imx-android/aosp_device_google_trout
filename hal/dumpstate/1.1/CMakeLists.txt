cmake_minimum_required(VERSION 3.13)

project(dumpstate_grpc_server CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    CACHE PATH "Path of static libraries.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    CACHE PATH "Path of dynamic libraries.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Path of executables.")
mark_as_advanced(CMAKE_RUNTIME_OUTPUT_DIRECTORY CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_ARCHIVE_OUTPUT_DIRECTORY)

set(_dumpstate_CXX_FLAGS -Wall -Werror -std=c++17)

set(_dumpstate_GRPCXX_LIBRARY_NAME "grpc++")

get_filename_component(dumpstate_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
set(dumpstate_GENS_DIR ${dumpstate_SRC_ROOT}/build/gens)

# ========== proto library ======================

set(dumpstate_PROTOBUF_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf)

if(EXISTS "${dumpstate_PROTOBUF_ROOT_DIR}/cmake/CMakeLists.txt")
    set(_dumpstate_PROTOBUF_WELLKNOWN_INCLUDE_DIR ${dumpstate_PROTOBUF_ROOT_DIR}/src)
else()
    message(FATAL_ERROR "${dumpstate_PROTOBUF_ROOT_DIR}/cmake/CMakeLists.txt not found")
endif()

# ========== grpc library ========================


set(dumpstate_GRPC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/grpc)

if(EXISTS "${dumpstate_GRPC_ROOT_DIR}/CMakeLists.txt")
  add_subdirectory(${dumpstate_GRPC_ROOT_DIR})
  if(TARGET ${_dumpstate_GRPCXX_LIBRARY_NAME})
    set(dumpstate_GRPCXX_LIBRARIES ${_dumpstate_GRPCXX_LIBRARY_NAME})
  endif()
else()
  message(FATAL_ERROR "${dumpstate_GRPC_ROOT_DIR}/CMakeLists.txt not found")
endif()

# ========== grpc proto prebuilts =================

find_program(dumpstate_PROTOC aprotoc REQUIRED)
find_program(dumpstate_GRPC_CPP_PLUGIN protoc-gen-grpc-cpp-plugin REQUIRED)

function(dumpstate_generate_grpc_cpp_from_proto)
  if(NOT ARGN)
    message(SEND_ERROR "Error: dumpstate_generate_grpc_cpp_from_proto() called without any proto files")
    return()
  endif()

  set(_protobuf_include_path -I . -I ${_dumpstate_PROTOBUF_WELLKNOWN_INCLUDE_DIR})

  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(ABS_DIR ${ABS_FIL} DIRECTORY)
    get_filename_component(FIL_NAME ${FIL} NAME)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    add_custom_command(
      OUTPUT "${dumpstate_GENS_DIR}/${FIL_WE}.grpc.pb.cc"
             "${dumpstate_GENS_DIR}/${FIL_WE}.grpc.pb.h"
             "${dumpstate_GENS_DIR}/${FIL_WE}.pb.cc"
             "${dumpstate_GENS_DIR}/${FIL_WE}.pb.h"
             COMMAND ${dumpstate_PROTOC}
      ARGS --grpc_out=generate_mock_code=true:${dumpstate_GENS_DIR}
           --cpp_out=${dumpstate_GENS_DIR}
           --plugin=protoc-gen-grpc=${dumpstate_GRPC_CPP_PLUGIN}
           ${_protobuf_include_path}
           ${FIL_NAME}
      DEPENDS ${ABS_FIL} ${dumpstate_PROTOC} ${dumpstate_GRPC_CPP_PLUGIN}
      WORKING_DIRECTORY ${ABS_DIR}
      COMMENT "Running gRPC C++ protocol buffer compiler on ${FIL}"
      VERBATIM)

      set_source_files_properties("${dumpstate_GENS_DIR}/${FIL_WE}.grpc.pb.cc" "${dumpstate_GENS_DIR}/${FIL_WE}.grpc.pb.h" "${dumpstate_GENS_DIR}/${FIL_WE}.pb.cc" "${dumpstate_GENS_DIR}/${FIL_WE}.pb.h" PROPERTIES GENERATED TRUE)
  endforeach()
endfunction()


dumpstate_generate_grpc_cpp_from_proto(
    ${dumpstate_SRC_ROOT}/proto/DumpstateServer.proto)

# ========== Protobuf generated code ==================

add_library(dumpstate_protobuf_gen
    ${dumpstate_GENS_DIR}/DumpstateServer.pb.cc
    ${dumpstate_GENS_DIR}/DumpstateServer.grpc.pb.cc
)

target_link_libraries(dumpstate_protobuf_gen
    ${dumpstate_GRPCXX_LIBRARIES}
)

target_compile_options(dumpstate_protobuf_gen PRIVATE ${_dumpstate_CXX_FLAGS} -Wno-array-bounds)

# ========== dumpstate_grpc_server ==============

add_executable(dumpstate_grpc_server
    ${dumpstate_SRC_ROOT}/DumpstateGrpcServer.cpp
    ${dumpstate_SRC_ROOT}/server_main.cpp
)

target_include_directories(dumpstate_grpc_server
    PRIVATE ${dumpstate_GENS_DIR}
)

target_link_libraries(dumpstate_grpc_server
    dumpstate_protobuf_gen
)

target_compile_options(dumpstate_grpc_server PRIVATE ${_dumpstate_CXX_FLAGS})

