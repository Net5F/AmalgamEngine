cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring MapGenerator")

add_executable(MapGenerator
    Private/MapGenerator.cpp
    Private/MapGenerator.h
    Private/MapGeneratorMain.cpp
)

target_include_directories(MapGenerator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(MapGenerator
    PRIVATE
        -static-libgcc
        -static-libstdc++
        Bitsery::bitsery
        Shared
)

# Compile with C++20
target_compile_features(MapGenerator PRIVATE cxx_std_20)
set_target_properties(MapGenerator PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
target_compile_options(MapGenerator PUBLIC -Wall -Wextra)
