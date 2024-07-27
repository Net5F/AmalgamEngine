cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Clock Test")

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
        ${SDL2_LIBRARIES}
        SDL2_net-static
        SharedLib
)
target_compile_features(ClockTestClient PRIVATE cxx_std_23)
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
        ${SDL2_LIBRARIES}
        SDL2_net-static
        SharedLib
)
target_compile_features(ClockTestServer PRIVATE cxx_std_23)
set_target_properties(ClockTestServer PROPERTIES CXX_EXTENSIONS OFF)
