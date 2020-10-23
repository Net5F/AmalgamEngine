#include <SDL2pp/SDL2pp.hh>

#include "Timer.h"
#include "Log.h"

#include "SimulatedClient.h"

#include <exception>
#include <atomic>
#include <vector>
#include <memory>
#include <string>

using namespace AM;
using namespace AM::LTC;

/** Default number of simulated clients if no argument is given. */
static constexpr unsigned int DEFAULT_NUM_CLIENTS = 10;

void logInvalidInput()
{
    LOG_ERROR("Invalid input.\n"
              "Usage: LoadTestClientMain.exe <number of clients>\n"
              "If no number of clients is given, will default to 10.");
}

void connectClients(unsigned int numClients,
                    std::vector<std::unique_ptr<SimulatedClient>>& clients)
{
    // Open all of the connections.
    for (unsigned int i = 0; i < numClients; ++i) {
        clients.push_back(std::make_unique<SimulatedClient>());
    }
}

int main(int argc, char** argv)
try {
    if (argc > 2) {
        logInvalidInput();
    }

    // Set up the SDL constructs.
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Amalgam Load Test Client", SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
                          SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Set up file logging.
    // TODO: This currently will do weird stuff if you have 2 clients open.
    Log::enableFileLogging("LoadTestClient.log");

    // Check for an argument for non-default number of clients.
    unsigned int numClients = DEFAULT_NUM_CLIENTS;
    if (argc > 1) {
        char* end;
        int input = std::strtol(argv[1], &end, 10);
        if ((*end != '\0') || (input < 1)) {
            // Input didn't parse into an integer, or integer was 0 or negative.
            logInvalidInput();
        }
        else {
            numClients = static_cast<unsigned int>(input);
        }
    }

    // Connect the clients.
    LOG_INFO("Connecting %u clients.", numClients);
    std::vector<std::unique_ptr<SimulatedClient>> clients;
    for (unsigned int i = 0; i < numClients; ++i) {
        clients.push_back(std::make_unique<SimulatedClient>());
    }
    LOG_INFO("%u clients connected.", numClients);

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

    return 0;
} catch (SDL2pp::Exception& e) {
    LOG_INFO("Error in: %s  Reason:  %s", e.GetSDLFunction(), e.GetSDLError());
    return 1;
} catch (std::exception& e) {
    LOG_INFO("%s", e.what());
    return 1;
}
