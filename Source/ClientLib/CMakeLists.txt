cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine Client Library")

# Add our static library target.
add_library(ClientLib STATIC "")

# Provide an alias that can be used whether the target comes from 
# add_subdirectory() or find_package().
add_library(AmalgamEngine::ClientLib ALIAS ClientLib)

target_include_directories(ClientLib
    PUBLIC
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_GFX_INCLUDE_DIRS} ${SDL2PP_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers as disabled for that target.
if (NOT APPLE OR CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
    target_precompile_headers(ClientLib REUSE_FROM SharedLib)
endif()

target_link_libraries(ClientLib
    PUBLIC
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_GFX_LIBRARIES} ${SDL2PP_LIBRARIES}
        SDL2pp
        AmalgamUI
        Bitsery::bitsery
        readerwriterqueue
        CircularBuffer
        EnTT::EnTT
        QueuedEvents
        SharedLib
)

# Compile with C++20.
target_compile_features(ClientLib PRIVATE cxx_std_20)
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
