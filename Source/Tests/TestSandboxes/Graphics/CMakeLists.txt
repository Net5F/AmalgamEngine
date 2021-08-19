cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Graphics Test Apps")

# Latency test client
add_executable(FrameTimeTest
    Private/FrameTimeTestMain.cpp
)

target_include_directories(FrameTimeTest
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(FrameTimeTest
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        Shared
)

target_compile_features(FrameTimeTest PRIVATE cxx_std_20)
set_target_properties(FrameTimeTest PROPERTIES CXX_EXTENSIONS OFF)
