cmake_minimum_required(VERSION 3.15)
project(kiosk C)

find_package(LIBPQ REQUIRED)

message("Building with CMake version: ${CMAKE_VERSION}")

add_executable(${PROJECT_NAME} src/main.cpp src/ticket.cpp src/card.cpp)

target_link_libraries(${PROJECT_NAME} LIPPQ::LIBPQ)