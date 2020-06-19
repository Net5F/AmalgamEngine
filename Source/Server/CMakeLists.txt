cmake_minimum_required(VERSION 3.16)

message(STATUS "Building Server")

add_executable(Server "")

target_include_directories(Server
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
target_precompile_headers(Server REUSE_FROM Shared)

target_link_libraries(Server
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        SDL2pp
        flatbuffers
        Shared
        readerwriterqueue
        CircularBuffer
)

# Compile with C++17
target_compile_features(Server PRIVATE cxx_std_17)
set_target_properties(Server PROPERTIES CXX_EXTENSIONS OFF)

# Build all of the subdirectories
add_subdirectory(Launch)
add_subdirectory(Game)
add_subdirectory(Utility)
