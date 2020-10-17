cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Drift Test")

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
