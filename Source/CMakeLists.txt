cmake_minimum_required(VERSION 3.13)

###############################################################################
# Modules
###############################################################################
add_subdirectory(SharedLib)

add_subdirectory(AccountLib)

add_subdirectory(ClientLib)

add_subdirectory(ServerLib)

add_subdirectory(AccountServer)

if (AM_BUILD_RESOURCE_IMPORTER)
    add_subdirectory(ResourceImporter)
endif()

if (AM_BUILD_TOOLS)
    add_subdirectory(Tools)
endif()

if (AM_BUILD_TESTS)
    add_subdirectory(Tests)
endif()
