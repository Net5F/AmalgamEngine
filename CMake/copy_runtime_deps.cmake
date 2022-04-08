cmake_minimum_required(VERSION 3.16)

if(${CMAKE_ARGC} LESS 5)
    message(FATAL_ERROR "Invalid arguments.\n"
      "Usage: cmake -P copy_runtime_deps.cmake <path_to_executable> <path_to_copy_to>\n"
      "On returning, the resolved dependencies will be copied into the directory at path_to_copy_to.\n"
      "Note: This script has an internal excludes list that can be modified if needed.")
endif()

# Normalize the environment path into cmake format.
file(TO_CMAKE_PATH "$ENV{PATH}" NORMALIZED_ENV_PATH)

# Set up our exclude list.
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    # Exclude all in system32.
    message(STATUS "Using Windows lib exclude regex.")
    set(EXCLUDE_SYSLIBS_REGEX [\\\/]WINDOWS[\\\/]system32\/*)
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    # Exclude all libs that were found to already come with Ubuntu 20.04.
    # Note: This produces a minimal list. If some of these libs are missing on some
    #       systems, they can be added back in.
    message(STATUS "Using Linux lib exclude regex.")
    set(EXCLUDE_SYSLIBS_REGEX ld- libbsd libc libdbus libdl
                              libgcrypt libgpg-err liblz4 liblzma libnsl
                              libpthread libresolv librt libsystemd libwrap
                              libXau libxcb libXdmcp libXext)
else()
    message(FATAL_ERROR "Unknown platform. Only Windows and Linux are supported.")
endif()

# Detect dependencies on the given executable.
message(STATUS "Detecting dependencies for: ${CMAKE_ARGV3}")
file(GET_RUNTIME_DEPENDENCIES
     RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
     EXECUTABLES ${CMAKE_ARGV3}
     DIRECTORIES ${NORMALIZED_ENV_PATH}
     POST_EXCLUDE_REGEXES ${EXCLUDE_SYSLIBS_REGEX}
)

# Copy dependencies into the given path.
message(STATUS "Copying dependencies to ${CMAKE_ARGV4}")
foreach(DEPENDENCY IN LISTS RESOLVED_DEPENDENCIES)
    file(COPY "${DEPENDENCY}" DESTINATION "${CMAKE_ARGV4}" FOLLOW_SYMLINK_CHAIN)
endforeach()
