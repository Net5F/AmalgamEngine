## The Amalgam Engine
### An engine for easily creating virtual worlds.
Project vision (not all implemented yet, see Status section)
* Easily create isometric, sprite-based virtual worlds.
* Start from a template and have a full working world, including client, server, text chat, and account management.
* All networking is provided, and adding new messages for your custom features is extremely easy.
* Targeted for use on relatively low-spec hardware (tested on a $30/mo rented server).
* Load tested with 150 clients in 1 spot or 1000 clients in groups of 10, all sending 4 inputs/second.
* Live, in-world map editing. Use permissions to let players build things, or to restrict it to your developers.

## Status
### Current
- [x] Implement, load test, and optimize the netcode.
- [x] Implement tile maps, iso rendering, and figure out architecture between the sim/rendering/UI.
- [x] Build minimal UI library.
- [x] Build sprite editor. (lets you add 3d bounding boxes to sprites for draw order and collision calcs)
- [x] Import sprite bounds data into engine.
- [x] Implement world map persistence.
- [x] Implement chunk and tile update streaming (live, in-world map editing).
- [x] Implement spatial partitioning grid for entities.
- [x] Implement interest management system.
- [x] Refactor UI library (spatial grid, nice event propagation).
- [x] Implement world editing UI.
- [x] Implement collision.
- [ ] Split single repo into engine and project repos.

### Future
- [ ] Implement text chat, chat server.
- [ ] Implement sprite animation.
- [ ] Implement login server, account db, account validation.
- [ ] Implement web server, account creation.
- [ ] MVP is done.

## Building
### Windows
I've been using MSYS2, but haven't yet scripted dependencies. Here's the list:

    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gdb mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_gfx mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-catch
    
Then, build through the Eclipse project or follow the linux instructions for a command line build.

### Linux
Had to update to Ubuntu 21.04 for g++ 10.3.0 (older versions didn't have support for some C++20 features I needed.)
I haven't yet locked down an SDL2 or CMake version, we'll see where it ends up.

1. Run `Scripts/Linux/InstallDependencies.sh`, then build through the Eclipse project, or:
2. (From the base of the repo) `mkdir Build Build/Linux Build/Linux/Debug`
3. `cd Build/Linux/Debug`
4. `cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -G Ninja ../../../Source/`
5. `ninja all`
