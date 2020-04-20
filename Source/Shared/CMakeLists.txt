cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Shared")

# Add our static library target
add_library(Shared INTERFACE)

target_include_directories(Shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Public)
