cmake_minimum_required(VERSION 3.13)

project(Amalgam)

# Tell CMake where the CMake folder is.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake/")

###############################################################################
# Options
###############################################################################
option(AM_BUILD_TESTS "Build Amalgam Engine tests." OFF)

###############################################################################
# Sanitizers 
###############################################################################
# Make ThreadSanitizer available.
OPTION(AM_THREADSANITIZER "Enable ThreadSanitizer data race detector." OFF)
include("${CMAKE_MODULE_PATH}/ThreadSanitizer.cmake")

# Make AddressSanitizer available.
OPTION(AM_ADDRESSSANITIZER "Enable AddressSanitizer memory error detector." OFF)
include("${CMAKE_MODULE_PATH}/AddressSanitizer.cmake")

###############################################################################
# Dependencies
###############################################################################
# Find SDL2 and associated libs.
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_gfx REQUIRED)
include("${CMAKE_MODULE_PATH}/SDL2_net.cmake")

# Configure lz4.
option(LZ4_BUILD_CLI "Build lz4 program" OFF)
option(LZ4_BUILD_LEGACY_LZ4C "Build lz4c program with legacy argument support" OFF)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/lz4/build/cmake/"
                 "${PROJECT_BINARY_DIR}/Libraries/lz4/")

# Configure SDL2pp.
set(SDL2PP_WITH_IMAGE ON)
set(SDL2PP_WITH_MIXER ON)
set(SDL2PP_WITH_TTF ON)
set(SDL2PP_WITH_NET ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/libSDL2pp/"
                 "${PROJECT_BINARY_DIR}/Libraries/libSDL2pp/")

# Configure bitsery.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/bitsery/"
                 "${PROJECT_BINARY_DIR}/Libraries/bitsery/")

# Configure Tracy.
option(TRACY_ENABLE "Enable Tracy profiling." OFF)
option(TRACY_ON_DEMAND "Wait for connection before starting profiling." ON)
# (Disabled because it requires extra dependencies. Can enable if useful.)
option(TRACY_NO_CALLSTACK "Disable all callstack related functionality" ON)
# (Below 2 are disabled because they didn't seem useful. Can enable if useful.)
option(TRACY_NO_VSYNC_CAPTURE "Disable capture of hardware Vsync events" ON)
option(TRACY_NO_FRAME_IMAGE  "Disable the frame image support and its thread" ON)
# (Disabled because it uses a lot of CPU in the remote server.)
option(TRACY_NO_SYSTEM_TRACING  "Disable systrace sampling" ON)
# (Disabled because the remote server doesn't support the default timers.)
option(TRACY_TIMER_FALLBACK "Use lower resolution timers" ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/tracy/"
                 "${PROJECT_BINARY_DIR}/Libraries/tracy/")

# Configure nlohmann json.
set(JSON_MultipleHeaders ON CACHE INTERNAL "Enable forward declaration.")
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/json/"
                 "${PROJECT_BINARY_DIR}/Libraries/json/")
                 
# Configure nativefiledialog-extended.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/nativefiledialog-extended/"
                 "${PROJECT_BINARY_DIR}/Libraries/nativefiledialog-extended/")

# Configure header-only readerwriterqueue.
include("${CMAKE_MODULE_PATH}/readerwriterqueue.cmake")

# Configure header-only CircularBuffer.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/CircularBuffer/"
                 "${PROJECT_BINARY_DIR}/Libraries/CircularBuffer/")

# Configure header-only QueuedEvents.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/QueuedEvents/"
                 "${PROJECT_BINARY_DIR}/Libraries/QueuedEvents/")

# Configure header-only EnTT.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/entt/"
                 "${PROJECT_BINARY_DIR}/Libraries/entt/")

# Configure AmalgamUI.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/AmalgamUI/"
                 "${PROJECT_BINARY_DIR}/Libraries/AmalgamUI/")

###############################################################################
# Source
###############################################################################
# Configure our project source.
add_subdirectory(Source)
