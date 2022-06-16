cmake_minimum_required(VERSION 3.13)

###############################################################################
# Modules
###############################################################################
# Configure shared library.
add_subdirectory(SharedLib)

# Configure client library.
add_subdirectory(ClientLib)

# Configure server library.
add_subdirectory(ServerLib)

# Configure sprite editor.
if (AM_BUILD_SPRITE_EDITOR)
    add_subdirectory(SpriteEditor)
endif()

# Configure tests.
if (AM_BUILD_TESTS)
    add_subdirectory(Tests)
endif()
