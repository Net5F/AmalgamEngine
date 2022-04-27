cmake_minimum_required(VERSION 3.16)

if(NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    message(FATAL_ERROR "Only Windows is supported.")
endif()

if(${CMAKE_ARGC} LESS 4)
    message(FATAL_ERROR "Invalid arguments.\n"
      "Usage: cmake -P copy_windows_deps.cmake <path_to_copy_to> <(Optional) is_headless>\n"
      "Attempts to find our dependencies by searching the environment PATH.\n"
      "  <path_to_copy_to>\n"
      "    The path to copy the resolved dependencies into.\n"
      "  <is_headless>\n"
      "    (Optional) If true, will only copy the server's dependencies. Else, will copy \n"
      "               the full list of dependencies.")
endif()

# Build our list of dependencies.
# Note: These are all SDL-related dependencies. The rest of our dependencies 
#       are statically linked.
set(HEADLESS_DEPENDENCIES SDL2.dll)
set(FULL_DEPENDENCIES SDL2.dll SDL2_gfx.dll SDL2_image.dll SDL2_mixer.dll SDL2_ttf.dll
                     libFLAC-8.dll libjpeg-9.dll libmodplug-1.dll libmpg123-0.dll 
                     libogg-0.dll libopus-0.dll libopusfile-0.dll libpng16-16.dll 
                     libtiff-5.dll libvorbis-0.dll libvorbisfile-3.dll libwebp-7.dll 
                     zlib1.dll)

set(IS_HEADLESS ${CMAKE_ARGV4})
if (${IS_HEADLESS})
    set(DLL_DEPENDENCIES ${HEADLESS_DEPENDENCIES})
else()
    set(DLL_DEPENDENCIES ${FULL_DEPENDENCIES})
endif()

# Resolve each DLL's location in the system.
foreach(DEPENDENCY_NAME IN LISTS DLL_DEPENDENCIES)
    # Try to find the file somewhere in the environment path.
    find_file(DLL_PATH
        ${DEPENDENCY_NAME}
        PATHS ENV PATH
        NO_CACHE
        REQUIRED)

    list(APPEND DLL_PATHS ${DLL_PATH})
    unset(DLL_PATH)
endforeach()

# Copy the resolved dependencies into the given path.
message(STATUS "Copying dependencies to ${CMAKE_ARGV3}")
foreach(DEPENDENCY IN LISTS DLL_PATHS)
    file(COPY "${DEPENDENCY}" DESTINATION "${CMAKE_ARGV3}" FOLLOW_SYMLINK_CHAIN)
endforeach()
