cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine Client Library")

# Add our static library target.
add_library(ClientLib STATIC "")

# Provide an alias that can be used whether the target comes from 
# add_subdirectory() or find_package().
add_library(AmalgamEngine::ClientLib ALIAS ClientLib)

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(ClientLib REUSE_FROM SharedLib)
endif()

target_link_libraries(ClientLib
    PUBLIC
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3_mixer::SDL3_mixer
        AmalgamUI
        Bitsery::bitsery
        readerwriterqueue
        CircularBuffer
        EnTT::EnTT
        QueuedEvents
        SharedLib
)

# Compile with C++23.
target_compile_features(ClientLib PRIVATE cxx_std_23)
set_target_properties(ClientLib PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ClientLib PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ClientLib PUBLIC /W3 /permissive-)
endif()

# Build all of the subdirectories.
add_subdirectory(Config)
add_subdirectory(Launch)
add_subdirectory(Network)
add_subdirectory(Renderer)
add_subdirectory(Simulation)
add_subdirectory(UserInterface)
add_subdirectory(Utility)
