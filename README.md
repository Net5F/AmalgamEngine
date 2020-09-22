## The Amalgam Engine
#### A game engine for building virtual worlds.

## Building
### Windows
I've been using MSYS2, but haven't yet scripted dependencies. Here's the list:

    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gdb mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-catch
    
Then, build through the Eclipse project or follow the linux instructions for a command line build.

### Linux
Tested on Ubuntu 20.0.4. Older distros may run into issues with package versions. I haven't yet locked down an SDL2 or CMake version, we'll see where it ends up.

1. Run `Scripts/Linux/InstallDependencies.sh`, then build through the Eclipse project, or:
2. (From the base of the repo) `mkdir Build Build/Linux Build/Linux/Debug`
3. `cd Build/Linux/Debug`
4. `cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -G Ninja ../../../Source/`
5. `ninja all`
