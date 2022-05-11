cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Shared")

# Add our static library target.
add_library(Shared STATIC "")

target_include_directories(Shared
    PUBLIC
        ${SDL2_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
)

target_link_libraries(Shared
    PUBLIC
        ${SDL2_LIBRARIES}
        ${SDL2PP_LIBRARIES}
        SDL2_net-static
        SDL2pp
        lz4_static
        EnTT::EnTT
        Bitsery::bitsery
        nlohmann_json::nlohmann_json
        Tracy::TracyClient
)

# Compile with C++20.
target_compile_features(Shared PRIVATE cxx_std_20)
set_target_properties(Shared PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(Shared PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Shared PUBLIC /W3 /permissive-)
endif()

# Build all of the subdirectories.
add_subdirectory(Config)
add_subdirectory(Messages)
add_subdirectory(Network)
add_subdirectory(Renderer)
add_subdirectory(Simulation)
add_subdirectory(Utility)
