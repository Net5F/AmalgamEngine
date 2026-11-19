#include "Log.h"
#include "Application.h"
#include "CommandLineTool.h"
#include "sodium.h"
#include <exception>
#include <span>
#include <SDL3/SDL_main.h>

using namespace AM;
using namespace AM::AccountServer;

// SDL needs this signature for main.
int main(int argc, char** argv)
try {
    // Set up file logging.
    Log::enableFileLogging("AccountServer.log");

    // Initialize libsodium.
    if (sodium_init() < 0) {
        LOG_FATAL("Failed to initialize libsodium.");
    }

    // If we were given a command, run it instead of the server.
    if (argc > 1) {
        CommandLineTool commandLineTool{
            std::span<char*>{argv + 1, static_cast<std::size_t>(argc - 1)}};
        return commandLineTool.run();
    }

    // Start the application (assumes control of the thread).
    Application app{};
    app.start();

    return 0;
} catch (std::exception& e) {
    LOG_INFO("%s", e.what());
    return 1;
}
