cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Shared")

# Add our static library target
add_library(Shared STATIC "")

target_include_directories(Shared
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
)

target_link_libraries(Shared
    PUBLIC
        ${SDL2_LIBRARIES}
        ${SDL2PP_LIBRARIES}
        SDL2_net-static
        SDL2pp
        Bitsery::bitsery
        Remotery
)

# Compile with C++17
target_compile_features(Shared PRIVATE cxx_std_17)
set_target_properties(Shared PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
target_compile_options(Shared PUBLIC -Wall -Wextra)

# Build all of the subdirectories
add_subdirectory(Messages)
add_subdirectory(Network)
add_subdirectory(Sim)
add_subdirectory(Utility)
