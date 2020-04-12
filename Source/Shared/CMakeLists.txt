cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Shared")

# Add our static library target
add_library(Shared STATIC
    Public/Message_generated.h
    Public/NWStructs.h
)

target_include_directories(Shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

# Compile with C++17
target_compile_features(Shared PRIVATE cxx_std_17)
set_target_properties(Shared PROPERTIES 
    CXX_EXTENSIONS OFF
    LINKER_LANGUAGE CXX
) 