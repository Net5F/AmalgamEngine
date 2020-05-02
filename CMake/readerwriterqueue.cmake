cmake_minimum_required(VERSION 3.5)

# Add our header-only library target
add_library(readerwriterqueue INTERFACE)

target_include_directories(readerwriterqueue
    INTERFACE
        ${PROJECT_SOURCE_DIR}/../Libraries/readerwriterqueue/
)

target_sources(readerwriterqueue INTERFACE
    ${PROJECT_SOURCE_DIR}/../Libraries/readerwriterqueue/readerwriterqueue.h
    ${PROJECT_SOURCE_DIR}/../Libraries/readerwriterqueue/atomicops.h
)
