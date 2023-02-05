#include "SDL.h"
#include "SDL_net.h"
#include <array>
#include <atomic>
#include <thread>
#include "Timer.h"
#include "Log.h"
#include "Ignore.h"
#include <iostream>

static constexpr int SERVER_PORT = 41499;

static constexpr double TEST_GAME_TICK_INTERVAL_S = 1 / 30.0;
/** An unreasonable amount of time for the game tick to be late by. */
static constexpr double TEST_GAME_DELAYED_TIME_S = .001;
static constexpr unsigned int NUM_BYTES = 55;

using namespace AM;

/** Exits if the user types exit. */
int inputThread(std::atomic<bool>* exitRequested)
{
    while (!(*exitRequested)) {
        std::string userInput = "";
        std::getline(std::cin, userInput);
        if (userInput == "exit") {
            *exitRequested = true;
        }
    }

    return 0;
}

/** Checks for new client connections, sends our current tick to new clients. */
void updateConnection(TCPsocket& serverSocket, TCPsocket& clientSocket,
                      SDLNet_SocketSet& clientSet, Uint32 currentTick)
{
    // Check for a connection.
    if (clientSocket == nullptr) {
        clientSocket = SDLNet_TCP_Accept(serverSocket);

        // If we got a connection, add the socket to the set.
        if (clientSocket != nullptr) {
            int numAdded = SDLNet_TCP_AddSocket(clientSet, clientSocket);
            if (numAdded < 1) {
                LOG_INFO("Error while adding socket: %s", SDLNet_GetError());
            }
            else {
                // Connected, send our current tick.
                // +1 because we'll have finished the tick by the time the
                // client gets this.
                Uint32 tickToSend = currentTick + 1;
                std::array<Uint8, sizeof(Uint32)> sendBuffer = {};
                _SDLNet_Write32(tickToSend, &sendBuffer);

                int bytesSent = SDLNet_TCP_Send(clientSocket, &sendBuffer,
                                                sizeof(Uint32));
                if (bytesSent < static_cast<int>(sizeof(Uint32))) {
                    LOG_FATAL("Failed to send current tick.");
                }

                LOG_INFO("Connected new client and sent current tick.");
            }
        }
    }
}

std::array<Uint8, NUM_BYTES> recBuffer = {};
// The number of bytes that we've received from the current message.
unsigned int bytesReceived = 0;
/** Receives the message from the client and compares the given tick number to
 * our current. */
bool receiveAndHandle(SDLNet_SocketSet& clientSet, TCPsocket& clientSocket,
                      std::atomic<Uint32>& currentTick)
{
    int numReady = SDLNet_CheckSockets(clientSet, 0);
    if (numReady == 1) {
        // Receive the waiting data.
        int result = SDLNet_TCP_Recv(clientSocket, &(recBuffer[bytesReceived]),
                                     (NUM_BYTES - bytesReceived));
        if (result <= 0) {
            LOG_INFO("Detected disconnect.");
            return false;
        }

        bytesReceived += result;

        // If we finished receiving the message, compare it to currentTick and
        // print the diff.
        if (bytesReceived == NUM_BYTES) {
            Sint64 clientTick = static_cast<Sint64>(_SDLNet_Read32(&recBuffer));
            Sint64 serverTick = static_cast<Sint64>(currentTick);

            LOG_INFO("Client tick: %d, Server tick: %d, Diff: %d", clientTick,
                     serverTick, (clientTick - serverTick));

            bytesReceived = 0;
        }
    }

    return true;
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

    /* Set up the listener. */
    IPaddress ip;
    TCPsocket serverSocket = nullptr;
    if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT)) {
        LOG_INFO("%s", SDLNet_GetError());
    }

    serverSocket = SDLNet_TCP_Open(&ip);
    if (!serverSocket) {
        LOG_INFO("%s", SDLNet_GetError());
    }

    TCPsocket clientSocket = nullptr;
    SDLNet_SocketSet clientSet = SDLNet_AllocSocketSet(1);

    /* Prepare the simulation variables. */
    // The aggregated time since we last processed a tick.
    double accumulatedTime = 0;
    // The number of the tick that we're currently on.
    std::atomic<Uint32> currentTick = 0;
    Debug::registerCurrentTickPtr(&currentTick);

    /* Spin up a thread to check for command line input. */
    std::atomic<bool> exitRequested = false;
    std::thread inputThreadObj(inputThread, &exitRequested);

    LOG_INFO("Server started.");

    // Prime a timer.
    Timer timer;
    while (!exitRequested) {
        // Connect or disconnect the client.
        updateConnection(serverSocket, clientSocket, clientSet, currentTick);

        // Calc the time delta.
        double deltaSeconds = timer.getTimeAndReset();

        /* Process as many game ticks as have accumulated. */
        accumulatedTime += deltaSeconds;
        while (accumulatedTime >= TEST_GAME_TICK_INTERVAL_S) {
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

        if (clientSocket != nullptr) {
            /* Try to receive. */
            bool result
                = receiveAndHandle(clientSet, clientSocket, currentTick);
            if (!result) {
                LOG_FATAL("Disconnect or other error during receive.");
            }
        }
    }

    inputThreadObj.join();

    return 0;
}
