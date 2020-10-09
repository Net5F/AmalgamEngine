#include "SDL.h"
#include "SDL_net.h"
#include <string>
#include <vector>
#include <array>
#include <atomic>
#include <cstdio>
#include "Timer.h"
#include "Log.h"
#include "Ignore.h"

const std::string SERVER_IP = "127.0.0.1";
// const std::string SERVER_IP = "45.79.37.63";
static constexpr unsigned int SERVER_PORT = 41499;

static constexpr double TEST_GAME_TICK_INTERVAL_S = 1 / 30.0;
/** An unreasonable amount of time for the game tick to be late by. */
static constexpr double TEST_GAME_DELAYED_TIME_S = .001;
static constexpr unsigned int NUM_BYTES = 55;

using namespace AM;

/** Waits for the server to send its current tick. */
bool waitForServer(TCPsocket& serverSocket, std::atomic<Uint32>& currentTick)
{
    std::array<Uint8, sizeof(Uint32)> tickBuf = {};

    bool messageReceived = false;
    while (!messageReceived) {
        int result = SDLNet_TCP_Recv(serverSocket, &tickBuf, sizeof(Uint32));
        if (result < 0) {
            // Disconnected
            LOG_INFO("Detected disconnect.");
            return false;
        }
        else if (result != sizeof(Uint32)) {
            LOG_INFO("Didn't receive current tick properly.");
            return false;
        }
        else {
            // +5 to put us in a reference frame ahead of the server.
            currentTick = _SDLNet_Read32(&tickBuf) + 5;
            Debug::info("Current tick received: %u", currentTick.load());
            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[])
{
    // SDL2 needs this signature for main, but we don't use the parameters.
    ignore(argc);
    ignore(argv);

    if (SDL_Init(0) == -1) {
        LOG_INFO("SDL_Init: %s", SDLNet_GetError());
        return 1;
    }
    if (SDLNet_Init() == -1) {
        LOG_INFO("SDLNet_Init: %s", SDLNet_GetError());
        return 2;
    }

    LOG_INFO("Connecting to server.");

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, SERVER_IP.c_str(), SERVER_PORT)) {
        LOG_INFO("Could not resolve host.");
        return 3;
    }

    TCPsocket serverSocket = SDLNet_TCP_Open(&ip);
    if (!serverSocket) {
        LOG_INFO("Could not open serverSocket.");
        return 4;
    }
    LOG_INFO("Connected.");

    /* Prepare the simulation variables. */
    // The aggregated time since we last processed a tick.
    double accumulatedTime = 0;
    // The number of the tick that we're currently on.
    std::atomic<Uint32> currentTick = 0;
    Debug::registerCurrentTickPtr(&currentTick);

    /* Prepare the data */
    // The data to send. The first 4 bytes will be replaced with the current
    // tick while running, the rest is filler.
    std::array<Uint8, NUM_BYTES> dataBuffer = {};
    for (unsigned int i = 0; i < NUM_BYTES; ++i) {
        dataBuffer[i] = NUM_BYTES % UINT8_MAX;
    }

    /* Wait to receive the server's current tick. */
    bool result = waitForServer(serverSocket, currentTick);
    if (!result) {
        LOG_ERROR("Failed to get server's current tick.");
    }

    // Prime the timer so it doesn't start at 0.
    Timer timer;
    timer.updateSavedTime();
    while (true) {
        // Calc the time delta.
        double deltaSeconds = timer.getDeltaSeconds(true);

        /* Process as many game ticks as have accumulated. */
        accumulatedTime += deltaSeconds;
        while (accumulatedTime >= TEST_GAME_TICK_INTERVAL_S) {
            /* Send our message. */
            _SDLNet_Write32(currentTick, &dataBuffer);
            int bytesSent
                = SDLNet_TCP_Send(serverSocket, &dataBuffer, NUM_BYTES);
            if (bytesSent < static_cast<int>(NUM_BYTES)) {
                LOG_INFO("Failed to send all bytes.");
                return 5;
            }

            /* Prepare for the next tick. */
            accumulatedTime -= TEST_GAME_TICK_INTERVAL_S;
            if (accumulatedTime >= TEST_GAME_TICK_INTERVAL_S) {
                LOG_INFO("Detected a request for multiple game ticks in the "
                         "same frame. Game tick "
                         "must have been massively delayed. Game tick was "
                         "delayed by: %.8fs.",
                         accumulatedTime);
            }
            else if (accumulatedTime >= TEST_GAME_DELAYED_TIME_S) {
                // Game missed its ideal call time, could be our issue or
                // general system slowness.
                LOG_INFO("Detected a delayed game tick. Game tick was delayed "
                         "by: %.8fs.",
                         accumulatedTime);
            }

            currentTick++;
        }
    }

    return 0;
}
