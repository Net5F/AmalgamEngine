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
add_subdirectory(SpriteEditor)

# Configure tests.
# (No fancy flags since we aren't yet a lib, just uncomment when you want to build tests.)
#add_subdirectory(Tests)

# Configure tools.
add_subdirectory(Tools)
