cmake_minimum_required(VERSION 3.13)

###############################################################################
# Modules
###############################################################################
# Configure shared library
add_subdirectory(Shared)

# Configure client
add_subdirectory(Client)

# Configure server
add_subdirectory(Server)

# Configure sprite editor
if (AM_BUILD_SPRITE_EDITOR)
    add_subdirectory(SpriteEditor)
endif()

# Configure tests.
if (AM_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

# Configure tools.
add_subdirectory(Tools)
