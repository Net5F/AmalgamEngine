cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Unit Tests")

# Catch2 settings.
SET(CATCH_BUILD_TESTING OFF CACHE BOOL "Build SelfTest project")
SET(CATCH_INSTALL_DOCS OFF CACHE BOOL "Install documentation alongside library")
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/Catch2/"
                 "${PROJECT_BINARY_DIR}/Libraries/Catch2/")

# Add the executable.
add_executable(UnitTests
    Private/TestMain.cpp
    Private/TestMessageSorter.cpp
    ${PROJECT_SOURCE_DIR}/Server/Utility/Private/MessageSorter.cpp
    ${PROJECT_SOURCE_DIR}/Server/Utility/Public/MessageSorter.h
)

# Include our source dir.
target_include_directories(UnitTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${PROJECT_SOURCE_DIR}/Server/Utility/Private
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Server/Utility/Public
)

# Link our dependencies.
target_link_libraries(UnitTests
    PRIVATE
        Shared
        Catch2::Catch2
)

# Compile with C++17
target_compile_features(UnitTests PRIVATE cxx_std_20)
set_target_properties(UnitTests PROPERTIES CXX_EXTENSIONS OFF)

# If debug, enable debug printing.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(UnitTests PUBLIC -DENABLE_DEBUG_INFO)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
