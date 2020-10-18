#include <SDL2pp/SDL2pp.hh>

#include "Peer.h"
#include "Timer.h"
#include "Log.h"
#include "Ignore.h"

#include "NetworkDefs.h"
#include "EntityUpdate.h"
#include "ConnectionResponse.h"
#include "ClientInputs.h"
#include "MessageTools.h"

#include <exception>
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <random>

using namespace AM;

static constexpr unsigned int SCREEN_WIDTH = 1280;
static constexpr unsigned int SCREEN_HEIGHT = 720;

static const std::string SERVER_IP = "45.79.37.63";
static constexpr unsigned int SERVER_PORT = 41499;

/** Default number of simulated clients if no argument is given. */
static constexpr unsigned int DEFAULT_NUM_CLIENTS = 10;

/** Two movements per second. */
static constexpr double TICK_TIMESTEP_S = (1 / 2.0);

/** Pre-serialized input messages. */
BinaryBufferSharedPtr leftInputMessage = nullptr;
BinaryBufferSharedPtr rightInputMessage = nullptr;

void serializeInputMessages()
{
    ClientInputs clientInputs{};
}

class Client
{
public:
    void tick()
    {
        accumulatedTime += iterationTimer.getDeltaSeconds(true);

        // If we're still waiting, don't do anything.
        if (initialWait > 0) {
            initialWait -= accumulatedTime;
            accumulatedTime = 0;
            return;
        }

        // If it's time, send an input.
        while (accumulatedTime >= TICK_TIMESTEP_S) {
            // TODO: Send the opposite input

            accumulatedTime -= TICK_TIMESTEP_S;
        }

        // TODO: Receive messages if any are waiting and do nothing with them.
    }

    std::unique_ptr<Peer> connection = nullptr;

    double initialWait = 0;
    Timer iterationTimer;

private:
    double accumulatedTime = 0;
    bool isMovingRight = false;
};

void logInvalidInput()
{
    LOG_ERROR("Invalid input.\n"
              "Usage: LoadTestClientMain.exe <number of clients>\n"
              "If no number of clients is given, will default to 10.");
}

void connectClients(unsigned int numClients, const std::vector<Client>& clients)
{
    // Init our random number generator.
    std::random_device randDevice;
    std::mt19937 randGenerator(randDevice);
    std::uniform_real_distribution<double> dist(0.0, 1000.0);

    // Open all of the connections.
    for (unsigned int i = 0; i < numClients; ++i) {
        clients[i].connection = Peer::initiate(SERVER_IP, SERVER_PORT);
        if (clients[i].connection != nullptr) {
            // Fill its initial wait.
            clients[i].initialWait = dist(randGenerator);

            // Init its timer.
            clients[i].iterationTimer.updateSavedTime();
        }
        else {
            LOG_ERROR("Failed to connect. i = %u", i);
        }
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

    // Uncomment to enable fullscreen.
    //    window.SetFullscreen(SDL_WINDOW_FULLSCREEN);

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
    std::vector<Client> clients;
    connectClients(numClients, clients);
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
            client.tick();
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
