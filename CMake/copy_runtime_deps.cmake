cmake_minimum_required(VERSION 3.16)

if(${CMAKE_ARGC} LESS 5)
    message(FATAL_ERROR "Invalid arguments.\n"
      "Usage: cmake -P get_runtime_deps.cmake <path_to_executable> <path_to_copy_to>\n"
      "On returning, the list of resolved dependencies will be copied into the directory at path_to_copy_to.")
endif()

# Normalize the environment path into cmake format.
file(TO_CMAKE_PATH "$ENV{PATH}" NORMALIZED_ENV_PATH)

# Detect dependencies on the given executable.
message(STATUS "Detecting dependencies for: ${CMAKE_ARGV3}")
file(GET_RUNTIME_DEPENDENCIES
     RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
     EXECUTABLES ${CMAKE_ARGV3}
     DIRECTORIES ${NORMALIZED_ENV_PATH}
     POST_EXCLUDE_REGEXES "[\\\/]WINDOWS[\\\/]system32\/*"
)

message(STATUS "Deps: ${RESOLVED_DEPENDENCIES}")

# Copy dependencies into the given path.
message(STATUS "Copying dependencies to ${CMAKE_ARGV4}")
foreach(DEPENDENCY IN LISTS RESOLVED_DEPENDENCIES)
    file(COPY "${DEPENDENCY}" DESTINATION "${CMAKE_ARGV4}" FOLLOW_SYMLINK_CHAIN)
endforeach()
