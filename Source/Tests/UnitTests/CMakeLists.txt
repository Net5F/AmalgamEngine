cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Amalgam Engine Unit Tests")

# Temp: Set to new behavior to avoid warning.
cmake_policy(SET CMP0135 NEW)

# Configure Catch2.
if(NOT TARGET Catch2::Catch2)
    message(STATUS "Downloading dependency if not present: Catch2")

    SET(CATCH_BUILD_TESTING OFF CACHE BOOL "Build SelfTest project")
    SET(CATCH_INSTALL_DOCS OFF CACHE BOOL "Install documentation alongside library")
    include(FetchContent)
    FetchContent_Declare(Catch2Download
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.3.1.tar.gz
        URL_HASH MD5=5cdc99f93e0b709936eb5af973df2a5c
    )
    FetchContent_MakeAvailable(Catch2Download)
endif()

# Add the executable.
add_executable(UnitTests
    Private/TestBoundingBox.cpp
    Private/TestEntityLocator.cpp
    Private/TestMain.cpp
)

# Include our source dir.
target_include_directories(UnitTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Source/ServerLib/Network/Public
)

# Link our dependencies.
target_link_libraries(UnitTests
    PRIVATE
        SharedLib
        Catch2::Catch2
)

# Compile with C++20
target_compile_features(UnitTests PRIVATE cxx_std_20)
set_target_properties(UnitTests PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(UnitTests PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(UnitTests PUBLIC /W3 /permissive-)
endif()

# If debug, enable debug printing.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(UnitTests PUBLIC -DENABLE_DEBUG_INFO)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
