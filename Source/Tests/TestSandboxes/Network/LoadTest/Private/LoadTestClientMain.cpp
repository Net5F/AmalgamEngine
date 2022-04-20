#include <SDL.h>
#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"
#include "SDL2pp/Exception.hh"

#include "Timer.h"
#include "Log.h"

#include "SimulatedClient.h"

#include <exception>
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <thread>

using namespace AM;
using namespace AM::LTC;

/** Default number of simulated clients if no argument is given. */
static constexpr unsigned int DEFAULT_NUM_CLIENTS = 10;

/** Default time to wait, in milliseconds, between connecting clients. */
static constexpr unsigned int DEFAULT_CONNECTION_WAIT_TIME_MS = 1;

void printUsage()
{
    std::printf(
        "Usage: LoadTestClientMain.exe <NumClients> <ConnectionWaitTime>\n"
        "NumClients: How many clients to simulate. Defaults to 10.\n"
        "ConnectionWaitTime: How long, in milliseconds, to wait between"
        " connecting clients. Defaults to 1ms.");
}

void connectClients(unsigned int numClients, unsigned int connectionWaitTimeMs,
                    std::vector<std::unique_ptr<SimulatedClient>>* clients)
{
    LOG_INFO("Connecting %u clients with a %ums wait time.", numClients,
             connectionWaitTimeMs);

    // Open all of the connections.
    for (unsigned int i = 0; i < numClients; ++i) {
        (*clients)[i]->connect();

        // Sleep for our wait time.
        std::this_thread::sleep_for(
            std::chrono::milliseconds(connectionWaitTimeMs));
    }

    LOG_INFO("%u clients connected.", numClients);
}

int main(int argc, char** argv)
try {
    if (argc > 3) {
        std::printf("Too many arguments.\n");
        printUsage();
        return 1;
    }

    // Set up the SDL constructs.
    SDL2pp::SDL sdl();

    // Set up file logging.
    // TODO: This currently will do weird stuff if you have 2 instances of this
    //       app open.
    //    Log::enableFileLogging("LoadTestClient.log");

    // Check for a NumClients argument.
    unsigned int numClients{DEFAULT_NUM_CLIENTS};
    if (argc > 1) {
        char* end;
        int input = std::strtol(argv[1], &end, 10);
        if ((*end != '\0') || (input < 1)) {
            // Input didn't parse into an integer, or value was less than 1.
            std::printf("Invalid input: %s\n", argv[1]);
            printUsage();
            return 1;
        }
        else {
            numClients = static_cast<unsigned int>(input);
        }
    }

    // Check for a ConnectionWaitTime argument.
    unsigned int connectionWaitTimeMs{DEFAULT_CONNECTION_WAIT_TIME_MS};
    if (argc > 2) {
        char* end;
        int input = std::strtol(argv[2], &end, 10);
        if ((*end != '\0') || (input < 1)) {
            // Input didn't parse into a float, or value was less than 1.
            std::printf("Invalid input: %s\n", argv[1]);
            printUsage();
            return 1;
        }
        else {
            connectionWaitTimeMs = static_cast<unsigned int>(input);
        }
    }

    // Construct the clients.
    std::vector<std::unique_ptr<SimulatedClient>> clients;
    for (unsigned int i = 0; i < numClients; ++i) {
        clients.push_back(std::make_unique<SimulatedClient>());
        clients[i]->setNetstatsLoggingEnabled(false);
    }

    // Enable only one client's logging so we don't get spammed.
    // Netstats logging is static, so it'll get the data from all clients.
    clients[0]->setNetstatsLoggingEnabled(true);

    // Start the client connections thread.
    std::thread connectionThreadObj(connectClients, numClients,
                                    connectionWaitTimeMs, &clients);

    // Start the main loop.
    std::atomic<bool> exitRequested = false;
    while (!exitRequested) {
        // Check for attempts to exit.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exitRequested = true;
            }
        }

        // Process the simulated clients.
        for (auto& client : clients) {
            client->tick();
        }
    }

    connectionThreadObj.join();

    return 0;
} catch (SDL2pp::Exception& e) {
    LOG_INFO("Error in: %s  Reason:  %s", e.GetSDLFunction().c_str(),
             e.GetSDLError().c_str());
    return 1;
} catch (std::exception& e) {
    LOG_INFO("%s", e.what());
    return 1;
}
