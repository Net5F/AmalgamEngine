cmake_minimum_required(VERSION 3.13)

project(Amalgam)

# Tell CMake where the CMake folder is, since our source dir isn't the base dir.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../CMake/")

# Find SDL2 and associated libs
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
include("${PROJECT_SOURCE_DIR}/../CMake/SDL2_net.cmake")

# Build static library dependency SDL2pp
set(SDL2PP_WITH_IMAGE ON)
set(SDL2PP_WITH_MIXER ON)
set(SDL2PP_WITH_TTF ON)
set(SDL2PP_WITH_NET ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/libSDL2pp/"
                 "${PROJECT_BINARY_DIR}/Libraries/libSDL2pp/")

# Build bitsery 
add_subdirectory(Lib/bitsery)

# Include header-only readerwriterqueue.
include("${PROJECT_SOURCE_DIR}/../CMake/readerwriterqueue.cmake")

# Include header-only CircularBuffer.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/CircularBuffer/"
                 "${PROJECT_BINARY_DIR}/Libraries/CircularBuffer/")

# Build shared library
add_subdirectory(Shared)

# Build client
add_subdirectory(Client)

# Build server
add_subdirectory(Server)

# Build editor
#add_subdirectory(Editor)

# Build tests.
# (No fancy flags since we aren't a lib, just uncomment when you want to build tests.)
#add_subdirectory(Tests)
