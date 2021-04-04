#include "Application.h"
#include "Ignore.h"
#include "Log.h"

#include "SDL2pp/Exception.hh"
#include <SDL_filesystem.h>

#include <exception>

using namespace AM;
using namespace AM::Client;

int main(int argc, char** argv)
try {
    // SDL2 needs this signature for main, but we don't use the parameters.
    ignore(argc);
    ignore(argv);

    // The path that this executable was ran from, excluding the binary name.
    std::string runPath{SDL_GetBasePath()};

    // Start the application (assumes control of the thread).
    Application app(runPath);
    app.start();

    return 0;
} catch (SDL2pp::Exception& e) {
    LOG_INFO("Error in: %s  Reason:  %s", e.GetSDLFunction().c_str(), e.GetSDLError().c_str());
    return 1;
} catch (std::exception& e) {
    LOG_INFO("%s", e.what());
    return 1;
}
