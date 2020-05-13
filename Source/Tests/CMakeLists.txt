cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Tests")

# Catch2 settings.
SET(CATCH_BUILD_TESTING OFF CACHE BOOL "Build SelfTest project")
SET(CATCH_INSTALL_DOCS OFF CACHE BOOL "Install documentation alongside library")
find_package( Catch2 REQUIRED )
include(CTest)

# Add the executable.
add_executable(Tests
    Private/TestMain.cpp
    Private/TestMessageSorter.cpp
    ${PROJECT_SOURCE_DIR}/Server/Utility/Private/MessageSorter.cpp
    ${PROJECT_SOURCE_DIR}/Server/Utility/Public/MessageSorter.h
)

# Include our source dir.
target_include_directories(Tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${PROJECT_SOURCE_DIR}/Server/Utility/Private
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Server/Utility/Public
)

# Link our dependencies.
target_link_libraries(Tests
    PRIVATE
        Shared
        Catch2::Catch2
)

# Compile with C++17
target_compile_features(Tests PRIVATE cxx_std_17)
set_target_properties(Tests PROPERTIES CXX_EXTENSIONS OFF)

# If debug, enable debug printing.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(Tests PUBLIC -DENABLE_DEBUG_INFO)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
