cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Network Tests")

# Latency test client
add_executable(LatencyTestClient
    Private/LatencyTestClientMain.cpp
)
target_include_directories(LatencyTestClient
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(LatencyTestClient
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        Shared
)
target_compile_features(LatencyTestClient PRIVATE cxx_std_17)
set_target_properties(LatencyTestClient PROPERTIES CXX_EXTENSIONS OFF)

# Latency test server
add_executable(LatencyTestServer
    Private/LatencyTestServerMain.cpp
)
target_include_directories(LatencyTestServer
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(LatencyTestServer
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        Shared
)
target_compile_features(LatencyTestServer PRIVATE cxx_std_17)
set_target_properties(LatencyTestServer PROPERTIES CXX_EXTENSIONS OFF)


# Throughput test client
add_executable(ThroughputTestClient
    Private/ThroughputTestClientMain.cpp
)
target_include_directories(ThroughputTestClient
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(ThroughputTestClient
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        Shared
)
target_compile_features(ThroughputTestClient PRIVATE cxx_std_17)
set_target_properties(ThroughputTestClient PROPERTIES CXX_EXTENSIONS OFF)

# Throughput test server
add_executable(ThroughputTestServer
    Private/ThroughputTestServerMain.cpp
)
target_include_directories(ThroughputTestServer
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(ThroughputTestServer
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        Shared
)
target_compile_features(ThroughputTestServer PRIVATE cxx_std_17)
set_target_properties(ThroughputTestServer PROPERTIES CXX_EXTENSIONS OFF)
