cmake_minimum_required(VERSION 3.13)

project(Amalgam)

# Tell CMake where the CMake folder is, since our source dir isn't the base dir.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../CMake/")

# Build SDL2.
set(SDL_SHARED OFF CACHE BOOL "Build a shared version of the library")
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/SDL/"
                 "${PROJECT_BINARY_DIR}/Libraries/SDL/")

# Build the SDL2 libs. They don't support cmake so we need to run our own.
include("${PROJECT_SOURCE_DIR}/../CMake/SDL2_image.cmake")
include("${PROJECT_SOURCE_DIR}/../CMake/SDL2_mixer.cmake")
include("${PROJECT_SOURCE_DIR}/../CMake/SDL2_net.cmake")
include("${PROJECT_SOURCE_DIR}/../CMake/SDL2_ttf.cmake")

# Build SDL2pp into a static lib.
set(SDL2PP_WITH_IMAGE ON)
set(SDL2PP_WITH_MIXER ON)
set(SDL2PP_WITH_TTF ON)
set(SDL2PP_WITH_NET ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/libSDL2pp/"
                 "${PROJECT_BINARY_DIR}/Libraries/libSDL2pp/")

# Build flatbuffers.
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "Enable the build of tests and samples." FORCE)
set(FLATBUFFERS_INSTALL OFF CACHE BOOL "Enable the installation of targets." FORCE)
set(FLATBUFFERS_BUILD_FLATHASH OFF CACHE BOOL "Enable the build of flathash" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/flatbuffers/"
                 "${PROJECT_BINARY_DIR}/Libraries/flatbuffers/")

# Include header-only readerwriterqueue.
include("${PROJECT_SOURCE_DIR}/../CMake/readerwriterqueue.cmake")

# Include header-only CircularBuffer.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/CircularBuffer/"
                 "${PROJECT_BINARY_DIR}/Libraries/CircularBuffer/")

# Build shared library.
add_subdirectory(Shared)

# Build client.
add_subdirectory(Client)

# Build server.
add_subdirectory(Server)

# Build editor.
#add_subdirectory(Editor)

# Build tests.
# (No fancy flags since we aren't a lib, just uncomment when you want to build tests.)
#add_subdirectory(Tests)
