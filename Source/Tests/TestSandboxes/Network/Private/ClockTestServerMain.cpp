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

using namespace AM;

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

    Timer timer;
    LOG_INFO("Server started.");

    LOG_INFO("Waiting for client connection.");
    while (clientSocket == nullptr) {
        clientSocket = SDLNet_TCP_Accept(serverSocket);

        // If we got a connection, add the socket to the set.
        if (clientSocket != nullptr) {
            int numAdded = SDLNet_TCP_AddSocket(clientSet, clientSocket);
            if (numAdded < 1) {
                LOG_INFO("Error while adding socket: %s", SDLNet_GetError());
            }
        }
    }

    // Wait for the start byte.
    LOG_INFO("Received client connection. Waiting for start byte.");
    std::array<Uint8, 1> recBuffer = {};
    bool waitingForStart = true;
    while (waitingForStart) {
        int numReady = SDLNet_CheckSockets(clientSet, 0);
        if (numReady == 1) {
            // Receive the waiting data.
            int result = SDLNet_TCP_Recv(clientSocket, &recBuffer, 1);
            if (result <= 0) {
                LOG_ERROR("Detected disconnect.");
            }
            else if (recBuffer[0] != 5) {
                LOG_ERROR("Wrong start byte received.");
            }
            else {
                // Got the start byte, start timing and proceed.
                timer.updateSavedTime();
                waitingForStart = false;
                LOG_INFO("Received start byte.");
            }
        }
    }

    // Wait for the end byte.
    bool waitingForEnd = true;
    while (waitingForEnd) {
        int numReady = SDLNet_CheckSockets(clientSet, 0);
        if (numReady == 1) {
            // Receive the waiting data.
            int result = SDLNet_TCP_Recv(clientSocket, &recBuffer, 1);
            if (result <= 0) {
                LOG_ERROR("Detected disconnect.");
            }
            else if (recBuffer[0] != 6) {
                LOG_ERROR("Wrong end byte received.");
            }
            else {
                LOG_INFO("Received end byte. Time passed: %.8f",
                          timer.getDeltaSeconds(false));
                waitingForEnd = false;
            }
        }
    }

    return 0;
}
