cmake_minimum_required(VERSION 3.13)

project(Amalgam)

# Tell CMake where the CMake folder is, since our source dir isn't the base dir.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake/")

# Make ThreadSanitizer available. Will only be used if AM_THREADSANITIZER == ON.
include("${CMAKE_MODULE_PATH}/ThreadSanitizer.cmake")

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

# Configure zlib-ng.
set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "Build test binaries." FORCE)
set(WITH_SANITIZER "" CACHE STRING "Build with sanitizer (Memory, Address, Thread, Undefined)." FORCE)
set(WITH_GZFILEOP OFF CACHE BOOL "Compile with support for .gz file saving/reading functions." FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/zlib-ng/"
                 "${PROJECT_BINARY_DIR}/Libraries/zlib-ng/")

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
option(TRACY_ENABLE "Enable Tracy profiling." ON)
option(TRACY_ON_DEMAND "Wait for connection before starting profiling." ON)
option(TRACY_NO_CALLSTACK "Disable all callstack related functionality" ON)
option(TRACY_NO_VSYNC_CAPTURE "Disable capture of hardware Vsync events" ON)
option(TRACY_NO_FRAME_IMAGE  "Disable the frame image support and its thread" ON)
option(TRACY_NO_CODE_TRANSFER "Disable collection of source code" ON)
option(TRACY_NO_CONTEXT_SWITCH "Disable capture of context switches" ON)
option(TRACY_NO_SAMPLING "Disable call stack sampling" ON)
option(TRACY_NO_VERIFY "Disable zone validation for C API" ON)
option(TRACY_NO_SYSTEM_TRACING  "Disable systrace sampling" ON)
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
