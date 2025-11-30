cmake_minimum_required(VERSION 4.2.0)

project(kiosk C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Protobuf setup
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

# gRPC setup
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

# Database dependencies
find_package(libpqxx REQUIRED)

find_package(nlohmann_json REQUIRED)

# Proto file setup
set(PROTO_ROOT "${CMAKE_SOURCE_DIR}/proto")
set(PROTO_GEN_ROOT "${PROTO_ROOT}/gen")
get_filename_component(kiosk_proto "${PROTO_ROOT}/kiosk/kiosk.proto" ABSOLUTE)
get_filename_component(kiosk_proto_path "${kiosk_proto}" PATH)

# Create proto gen directory if it doesn't exist
file(MAKE_DIRECTORY "${PROTO_GEN_ROOT}/kiosk")

# Generated sources
set(kiosk_proto_srcs "${PROTO_GEN_ROOT}/kiosk/kiosk.pb.cc")
set(kiosk_proto_hdrs "${PROTO_GEN_ROOT}/kiosk/kiosk.pb.h")
set(kiosk_grpc_srcs "${PROTO_GEN_ROOT}/kiosk/kiosk.grpc.pb.cc")
set(kiosk_grpc_hdrs "${PROTO_GEN_ROOT}/kiosk/kiosk.grpc.pb.h")

# Proto generation command
add_custom_command(
  OUTPUT "${kiosk_proto_srcs}" "${kiosk_proto_hdrs}" "${kiosk_grpc_srcs}" "${kiosk_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${PROTO_GEN_ROOT}/kiosk"
       --cpp_out "${PROTO_GEN_ROOT}/kiosk"
       -I "${kiosk_proto_path}"
       --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
       "${kiosk_proto}"
  DEPENDS "${kiosk_proto}"
  COMMENT "Generating gRPC and Protobuf files"
)

# Include generated files
include_directories("${PROTO_GEN_ROOT}")

# gRPC proto library
add_library(kiosk_grpc_proto
  ${kiosk_grpc_srcs}
  ${kiosk_grpc_hdrs}
  ${kiosk_proto_srcs}
  ${kiosk_proto_hdrs}
)

target_link_libraries(kiosk_grpc_proto
  PUBLIC
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    libpqxx::pqxx
)

# Core libraries
set(CORE_LIBRARIES
  db
)

foreach(lib ${CORE_LIBRARIES})
  add_library(${lib}
    "${CMAKE_SOURCE_DIR}/src/${lib}.h"
    "${CMAKE_SOURCE_DIR}/src/${lib}.cc"
  )
  target_link_libraries(${lib}
    PUBLIC
      kiosk_grpc_proto
      ${_REFLECTION}
      ${_GRPC_GRPCPP}
      ${_PROTOBUF_LIBPROTOBUF}
      libpqxx::pqxx
      nlohmann_json::nlohmann_json
  )
endforeach()

# Main executable
add_executable(kiosk_server 
  "${CMAKE_SOURCE_DIR}/src/kiosk_server.cc"
)

target_link_libraries(kiosk_server
  PRIVATE
    kiosk_grpc_proto
    db
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    libpqxx::pqxx
    nlohmann_json::nlohmann_json
)

# Testing setup
enable_testing()
find_package(GTest REQUIRED)

# Test executable
add_executable(db_test
  tests/db_test.cc
)

target_link_libraries(db_test
  PRIVATE
    db
    GTest::GTest
    GTest::Main
)

add_test(NAME db_test COMMAND db_test)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)