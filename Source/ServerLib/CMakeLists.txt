cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine Server Library")

# Add our static library target.
add_library(ServerLib STATIC "")

# Provide an alias that can be used whether the target comes from 
# add_subdirectory() or find_package().
add_library(AmalgamEngine::ServerLib ALIAS ServerLib)

target_include_directories(ServerLib
    PUBLIC
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(ServerLib REUSE_FROM SharedLib)
endif()

target_link_libraries(ServerLib
    PUBLIC
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2PP_LIBRARIES}
        SDL2_net-static
        SDL2pp
        boost_mp11
        Bitsery::bitsery
        readerwriterqueue
        CircularBuffer
        EnTT::EnTT
        Lua
        sol2::sol2
        QueuedEvents
        SharedLib
        Tracy::TracyClient
)

# Compile with C++20.
target_compile_features(ServerLib PRIVATE cxx_std_20)
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
