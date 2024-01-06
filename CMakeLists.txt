cmake_minimum_required(VERSION 3.15)
project(kiosk)

find_package(PostgreSQL REQUIRED)

message("Building with CMake version: ${CMAKE_VERSION}")

add_executable(${PROJECT_NAME} src/main.cpp src/ticket.cpp src/card.cpp src/movies.cpp src/helper.hpp)

target_link_libraries(${PROJECT_NAME} PostgreSQL::PostgreSQL)