cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Remotery")

# Build options
OPTION(ENABLE_PROFILING "Enable Remotery profiling." OFF)

# Add our static library target
add_library(Remotery STATIC
       ${PROJECT_SOURCE_DIR}/../Libraries/Remotery/lib/Remotery.c
)

target_include_directories(Remotery
    PUBLIC
        ${PROJECT_SOURCE_DIR}/../Libraries/Remotery/lib
)

if(MINGW)
    target_link_libraries(Remotery PRIVATE ws2_32)
endif()

if (ENABLE_PROFILING)
    target_compile_definitions(Remotery PUBLIC PROFILING_ENABLED)
endif()
