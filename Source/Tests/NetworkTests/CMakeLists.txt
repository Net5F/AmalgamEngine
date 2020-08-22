cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Network Tests")

# Latency test client
add_executable(LatencyTestClient
    Private/LatencyTestClientMain.cpp
)
target_include_directories(LatencyTestClient
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(LatencyTestClient
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        SDL2_net-static
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
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(LatencyTestServer
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        SDL2_net-static
        Shared
)
target_compile_features(LatencyTestServer PRIVATE cxx_std_17)
set_target_properties(LatencyTestServer PROPERTIES CXX_EXTENSIONS OFF)


# Drift test client
add_executable(DriftTestClient
    Private/DriftTestClientMain.cpp
)
target_include_directories(DriftTestClient
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(DriftTestClient
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        SDL2_net-static
        Shared
)
target_compile_features(DriftTestClient PRIVATE cxx_std_17)
set_target_properties(DriftTestClient PROPERTIES CXX_EXTENSIONS OFF)

# Drift test server
add_executable(DriftTestServer
    Private/DriftTestServerMain.cpp
)
target_include_directories(DriftTestServer
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(DriftTestServer
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        SDL2_net-static
        Shared
)
target_compile_features(DriftTestServer PRIVATE cxx_std_17)
set_target_properties(DriftTestServer PROPERTIES CXX_EXTENSIONS OFF)


# Clock test client
add_executable(ClockTestClient
    Private/ClockTestClientMain.cpp
)
target_include_directories(ClockTestClient
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(ClockTestClient
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        SDL2_net-static
        Shared
)
target_compile_features(ClockTestClient PRIVATE cxx_std_17)
set_target_properties(ClockTestClient PROPERTIES CXX_EXTENSIONS OFF)

# Clock test server
add_executable(ClockTestServer
    Private/ClockTestServerMain.cpp
)
target_include_directories(ClockTestServer
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
target_link_libraries(ClockTestServer
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES}
        SDL2_net-static
        Shared
)
target_compile_features(ClockTestServer PRIVATE cxx_std_17)
set_target_properties(ClockTestServer PROPERTIES CXX_EXTENSIONS OFF)
