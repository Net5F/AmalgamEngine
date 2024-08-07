cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Latency Test")

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
        ${SDL2_LIBRARIES}
        SDL2_net-static
        SharedLib
)
target_compile_features(LatencyTestClient PRIVATE cxx_std_23)
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
        ${SDL2_LIBRARIES}
        SDL2_net-static
        SharedLib
)
target_compile_features(LatencyTestServer PRIVATE cxx_std_23)
set_target_properties(LatencyTestServer PROPERTIES CXX_EXTENSIONS OFF)
