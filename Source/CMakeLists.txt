cmake_minimum_required(VERSION 3.13)

###############################################################################
# Modules
###############################################################################
# Configure Shared library.
add_subdirectory(SharedLib)

# Configure Client library.
add_subdirectory(ClientLib)

# Configure Server library.
add_subdirectory(ServerLib)

# Configure Resource Importer.
if (AM_BUILD_RESOURCE_IMPORTER)
    add_subdirectory(ResourceImporter)
endif()

# Configure Tools.
if (AM_BUILD_TOOLS)
    add_subdirectory(Tools)
endif()

# Configure tests.
if (AM_BUILD_TESTS)
    add_subdirectory(Tests)
endif()
