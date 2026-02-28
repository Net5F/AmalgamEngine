cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine Server Library")

# Add our static library target.
add_library(ServerLib STATIC "")

# Provide an alias that can be used whether the target comes from 
# add_subdirectory() or find_package().
add_library(AmalgamEngine::ServerLib ALIAS ServerLib)

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(ServerLib REUSE_FROM SharedLib)
endif()

target_link_libraries(ServerLib
    PUBLIC
        SDL3::SDL3
        SDL3_net::SDL3_net
        boost_mp11
        Bitsery::bitsery
        readerwriterqueue
        CircularBuffer
        EnTT::EnTT
        Lua
        sol2::sol2
        SQLiteCpp
        sqlite3
        QueuedEvents
        SharedLib
        Tracy::TracyClient
)

# Compile with C++23.
target_compile_features(ServerLib PRIVATE cxx_std_23)
set_target_properties(ServerLib PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ServerLib PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ServerLib PUBLIC /W3 /permissive-)
endif()

# Build all of the subdirectories.
add_subdirectory(Config)
add_subdirectory(Launch)
add_subdirectory(Network)
add_subdirectory(Simulation)
