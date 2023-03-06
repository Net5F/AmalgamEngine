cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Amalgam Engine Load Test Client")

# Load test client
add_executable(LoadTestClient
    Private/LoadTestClientMain.cpp
    Private/SimulatedClient.cpp
    Public/SimulatedClient.h
    Private/WorldSimulation.cpp
    Public/WorldSimulation.h
    
    # Client objects
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Config/Public/Config.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Config/Private/UserConfig.cpp
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Config/Public/UserConfig.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public/ConnectionError.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Private/MessageProcessor.cpp
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public/MessageProcessor.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Private/Network.cpp
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public/Network.h
)

target_include_directories(LoadTestClient
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public

        # Client objects
        ${PROJECT_SOURCE_DIR}/Source/ClientLib/Config/Public
        ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public
)

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(LoadTestClient REUSE_FROM SharedLib)
endif()

target_link_libraries(LoadTestClient
    PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2PP_LIBRARIES}
        SDL2_net-static
        readerwriterqueue
        EnTT::EnTT
        QueuedEvents
        SharedLib
)

# Compile with C++20
target_compile_features(LoadTestClient PRIVATE cxx_std_20)
set_target_properties(LoadTestClient PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(LoadTestClient PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(LoadTestClient PUBLIC /W3 /permissive-)
endif()

# Copy UserConfig.json to the build directory.
file(COPY ${CMAKE_SOURCE_DIR}/Resources/Client/Common/UserConfig.json
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# On Windows, copy the SDL2 DLL into the build folder so we can run our executable.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    file(COPY ${SDL2_DIR}/lib/x64/SDL2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
endif()
