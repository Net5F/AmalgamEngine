#include "Log.h"
#include "Application.h"
#include <exception>
#include <SDL3/SDL_main.h>

using namespace AM;
using namespace AM::ResourceImporter;

// SDL needs this signature for main.
int main(int, char**)
try {
    // Set up file logging.
    Log::enableFileLogging("ResourceImporter.log");

    // Start the application (assumes control of the thread).
    Application app;
    app.start();

    return 0;
} catch (std::exception& e) {
    LOG_INFO("%s", e.what());
    return 1;
}
