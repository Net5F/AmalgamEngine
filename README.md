## The Amalgam Engine
#### An engine for creating virtual worlds.
This engine aims to make it easy to create 2D isometric virtual worlds. It'll be ready when you're able to pull it down, build it, and immediately have a working world that you can start building on top of.

Everything you need will be provided, including:
* Client
* Server
* Minimal editor (most editing happens in-world)
* Login server
* Web server (to host the website where users can register accounts)
* Text chat server

## Status
(Once things are more ready for outside developers, I'll link the trello.)
### Current
- [x] Implement, load test, optimize the netcode.
- [x] Implement tile maps, iso rendering, and figure out architecture between the sim/rendering/UI.
- [x] Build minimal UI library. (MVP working, needs refactoring later)
- [x] Build sprite editor. (lets you add 3d bounding boxes to sprites for draw order and collision calcs)
- [x] Import sprite bounds data into engine, implement world map file.
- [x] Implement chunk and tile update streaming (in-world map editing).
- [x] Implement spatial partitioning.
- [ ] Implement world editing UI.

### Future
- [ ] Implement collision.
- [ ] Implement sprite animation.
- [ ] Implement text chat, chat server.
- [ ] Implement login server, account db, account validation.
- [ ] Implement web server, account creation.
- [ ] Implement minimal UI for sprite selection from a set list.
- [ ] Split single repo into engine and project repos.
- [ ] Ready for other people to use.

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
