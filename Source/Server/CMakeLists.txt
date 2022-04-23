cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Server")

add_executable(Server "")

target_include_directories(Server
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
target_precompile_headers(Server REUSE_FROM Shared)

target_link_libraries(Server
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-static-libgcc>
        $<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2PP_LIBRARIES}
        SDL2_net-static
        SDL2pp
        Bitsery::bitsery
        readerwriterqueue
        CircularBuffer
        EnTT::EnTT
        QueuedEvents
        Shared
)

# Compile with C++20
target_compile_features(Server PRIVATE cxx_std_20)
set_target_properties(Server PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(Server PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Server PUBLIC /W3 /permissive-)
endif()

# Copy SpriteData.json to the build directory.
file(COPY ${PROJECT_SOURCE_DIR}/Resources/Shared/SpriteData.json
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# Copy TileMap.bin to the build directory.
file(COPY ${PROJECT_SOURCE_DIR}/Resources/Server/TileMap.bin
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# Build all of the subdirectories
add_subdirectory(Config)
add_subdirectory(Launch)
add_subdirectory(Network)
add_subdirectory(Simulation)
add_subdirectory(Utility)
