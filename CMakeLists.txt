cmake_minimum_required(VERSION 3.15)

project(kiosk C CXX)

find_package(Threads REQUIRED)

option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

# Proto file
get_filename_component(kiosk_proto "protos/kiosk.proto" ABSOLUTE)
get_filename_component(kiosk_proto_path "${kiosk_proto}" PATH)

# Generated sources
set(kiosk_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/kiosk.pb.cc")
set(kiosk_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/kiosk.pb.h")
set(kiosk_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/kiosk.grpc.pb.cc")
set(kiosk_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/kiosk.grpc.pb.h")
add_custom_command(
      OUTPUT "${kiosk_proto_srcs}" "${kiosk_proto_hdrs}" "${kiosk_grpc_srcs}" "${kiosk_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${kiosk_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${kiosk_proto}"
      DEPENDS "${kiosk_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# m_grpc_proto
add_library(kiosk_grpc_proto
  ${kiosk_grpc_srcs}
  ${kiosk_grpc_hdrs}
  ${kiosk_proto_srcs}
  ${kiosk_proto_hdrs})
target_link_libraries(kiosk_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# kiosk_helper
# add_library(kiosk_helper
#   "helper.h"
#   "helper.cc")
# target_link_libraries(kiosk_helper
#   m_grpc_proto
#   ${_REFLECTION}
#   ${_GRPC_GRPCPP}
#   ${_PROTOBUF_LIBPROTOBUF})

# Targets kiosk_(client|server)
foreach(_target
  kiosk_client kiosk_server)
  add_executable(${_target}
    "../src/${_target}.cc")
  target_link_libraries(${_target}
    kiosk_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()