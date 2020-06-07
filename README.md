## The Amalgam Engine
#### A game engine for virtual worlds.

## Building
### Linux
Tested on Ubuntu 20.0.4. Older distros may run into issues with package versions. I haven't yet locked down an SDL2 or CMake version, we'll see where it ends up.

1. Run `Scripts/Linux/InstallDependencies.sh`
2. (From the base of the repo) `mkdir Build Build/Linux Build/Linux/Debug`
3. `cd Build/Linux/Debug`
4. `cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -G Ninja ../../../Source/`
5. `ninja all`
