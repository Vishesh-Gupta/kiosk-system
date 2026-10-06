cmake_minimum_required(VERSION 3.22)

project(kiosk C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(KIOSK_BUILD_TESTS "Build the kiosk test suite" ON)
option(ENABLE_COVERAGE "Instrument the build for gcov/lcov coverage" OFF)

if(ENABLE_COVERAGE)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

find_package(Threads REQUIRED)

# --- Dependencies -----------------------------------------------------------
# Prefer the CMake config packages (Conan's CMakeDeps or a source install of
# gRPC); fall back to CMake's FindProtobuf module for distro protobuf builds.
find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
  find_package(Protobuf REQUIRED)
endif()
message(STATUS "Using protobuf ${Protobuf_VERSION}")

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

find_package(libpqxx REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ftxui REQUIRED)

# Code generators run on the build machine. Use the imported targets when the
# package exports them, otherwise look them up on the program path (Conan's
# toolchain adds tool_requires to CMAKE_PROGRAM_PATH).
if(TARGET protobuf::protoc AND NOT CMAKE_CROSSCOMPILING)
  set(KIOSK_PROTOC $<TARGET_FILE:protobuf::protoc>)
else()
  find_program(KIOSK_PROTOC protoc REQUIRED)
endif()

if(TARGET gRPC::grpc_cpp_plugin AND NOT CMAKE_CROSSCOMPILING)
  set(KIOSK_GRPC_CPP_PLUGIN $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else()
  find_program(KIOSK_GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)
endif()

# --- Generated protobuf / gRPC code -----------------------------------------
set(PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_GEN_ROOT "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(kiosk_proto "${PROTO_ROOT}/kiosk/kiosk.proto")

set(kiosk_proto_srcs "${PROTO_GEN_ROOT}/kiosk/kiosk.pb.cc")
set(kiosk_proto_hdrs "${PROTO_GEN_ROOT}/kiosk/kiosk.pb.h")
set(kiosk_grpc_srcs "${PROTO_GEN_ROOT}/kiosk/kiosk.grpc.pb.cc")
set(kiosk_grpc_hdrs "${PROTO_GEN_ROOT}/kiosk/kiosk.grpc.pb.h")

file(MAKE_DIRECTORY "${PROTO_GEN_ROOT}")

add_custom_command(
  OUTPUT "${kiosk_proto_srcs}" "${kiosk_proto_hdrs}" "${kiosk_grpc_srcs}" "${kiosk_grpc_hdrs}"
  COMMAND ${KIOSK_PROTOC}
  ARGS --grpc_out "${PROTO_GEN_ROOT}"
       --cpp_out "${PROTO_GEN_ROOT}"
       -I "${PROTO_ROOT}"
       --plugin=protoc-gen-grpc=${KIOSK_GRPC_CPP_PLUGIN}
       "${kiosk_proto}"
  DEPENDS "${kiosk_proto}"
  COMMENT "Generating gRPC and Protobuf sources for kiosk.proto"
  VERBATIM
)

add_library(kiosk_grpc_proto
  ${kiosk_proto_srcs}
  ${kiosk_grpc_srcs}
)
target_include_directories(kiosk_grpc_proto PUBLIC "${PROTO_GEN_ROOT}")
target_link_libraries(kiosk_grpc_proto
  PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
)

# --- Libraries --------------------------------------------------------------
add_library(db src/db.cc)
target_include_directories(db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(db
  PUBLIC libpqxx::pqxx
  PRIVATE nlohmann_json::nlohmann_json
)

add_library(kiosk_service src/kiosk_service.cc)
target_link_libraries(kiosk_service
  PUBLIC
    db
    kiosk_grpc_proto
)

# --- Server -----------------------------------------------------------------
add_executable(kiosk_server src/kiosk_server.cc)
target_link_libraries(kiosk_server
  PRIVATE
    kiosk_service
    gRPC::grpc++_reflection
    Threads::Threads
)

# --- Terminal client --------------------------------------------------------
add_library(kiosk_client src/kiosk_client.cc)
target_include_directories(kiosk_client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(kiosk_client PUBLIC kiosk_grpc_proto)

add_library(kiosk_tui_lib src/kiosk_tui.cc)
target_link_libraries(kiosk_tui_lib
  PUBLIC
    kiosk_client
    ftxui::component
    Threads::Threads
)

add_executable(kiosk_tui src/kiosk_tui_main.cc)
target_link_libraries(kiosk_tui PRIVATE kiosk_tui_lib)

install(TARGETS kiosk_server kiosk_tui RUNTIME DESTINATION bin)

# --- Tests ------------------------------------------------------------------
if(KIOSK_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include(GoogleTest)

  foreach(test_name db_test kiosk_service_test kiosk_tui_test)
    add_executable(${test_name} tests/${test_name}.cc)
    target_link_libraries(${test_name}
      PRIVATE
        kiosk_service
        kiosk_tui_lib
        GTest::gtest
        GTest::gtest_main
    )
    gtest_discover_tests(${test_name} DISCOVERY_MODE PRE_TEST)
  endforeach()
endif()
