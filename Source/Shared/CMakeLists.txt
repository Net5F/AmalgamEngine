cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Shared")

# Add our static library target
add_library(Shared STATIC "")

target_include_directories(Shared
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(Shared
    PUBLIC
        ${SDL2_LIBRARIES}
        SDL2_net-static
        SDL2pp
        Bitsery::bitsery
        spdlog::spdlog
)

# Compile with C++17
target_compile_features(Shared PRIVATE cxx_std_17)
set_target_properties(Shared PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
target_compile_options(Shared PUBLIC -Wall -Wextra)

# If debug, enable debug printing.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(Shared PUBLIC -DENABLE_DEBUG_INFO)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Build all of the subdirectories
add_subdirectory(Components)
add_subdirectory(Messages)
add_subdirectory(Network)
add_subdirectory(Sim)
add_subdirectory(Utility)
