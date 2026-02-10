#include "Log.h"
#include "Application.h"

#include <exception>

using namespace AM;
using namespace AM::ResourceImporter;

// SDL2 needs this signature for main, but we don't use the parameters.
int main(int, char**)
try {
    // Set up file logging.
    Log::enableFileLogging("ResourceImporter.log");

    // Start the application (assumes control of the thread).
    Application app;
    app.start();

    return 0;
}  catch (std::exception& e) {
    LOG_INFO("%s", e.what());
    return 1;
}
