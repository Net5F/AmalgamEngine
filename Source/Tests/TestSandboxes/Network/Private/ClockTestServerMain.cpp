#include "SDL.h"
#include "SDL_net.h"
#include <array>
#include <atomic>
#include <thread>
#include "Timer.h"
#include "Debug.h"
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
        DebugInfo("SDL_Init: %s", SDLNet_GetError());
        return 1;
    }
    if (SDLNet_Init() == -1) {
        DebugInfo("SDLNet_Init: %s", SDLNet_GetError());
        return 2;
    }

    /* Set up the listener. */
    IPaddress ip;
    TCPsocket serverSocket = nullptr;
    if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT)) {
        DebugInfo("%s", SDLNet_GetError());
    }

    serverSocket = SDLNet_TCP_Open(&ip);
    if (!serverSocket) {
        DebugInfo("%s", SDLNet_GetError());
    }

    TCPsocket clientSocket = nullptr;
    SDLNet_SocketSet clientSet = SDLNet_AllocSocketSet(1);

    Timer timer;
    DebugInfo("Server started.");

    DebugInfo("Waiting for client connection.");
    while (clientSocket == nullptr) {
        clientSocket = SDLNet_TCP_Accept(serverSocket);

        // If we got a connection, add the socket to the set.
        if (clientSocket != nullptr) {
            int numAdded = SDLNet_TCP_AddSocket(clientSet, clientSocket);
            if (numAdded < 1) {
                DebugInfo("Error while adding socket: %s", SDLNet_GetError());
            }
        }
    }

    // Wait for the start byte.
    DebugInfo("Received client connection. Waiting for start byte.");
    std::array<Uint8, 1> recBuffer = {};
    bool waitingForStart = true;
    while (waitingForStart) {
        int numReady = SDLNet_CheckSockets(clientSet, 0);
        if (numReady == 1) {
            // Receive the waiting data.
            int result = SDLNet_TCP_Recv(clientSocket, &recBuffer, 1);
            if (result <= 0) {
                DebugError("Detected disconnect.");
            }
            else if (recBuffer[0] != 5) {
                DebugError("Wrong start byte received.");
            }
            else {
                // Got the start byte, start timing and proceed.
                timer.updateSavedTime();
                waitingForStart = false;
                DebugInfo("Received start byte.");
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
                DebugError("Detected disconnect.");
            }
            else if (recBuffer[0] != 6) {
                DebugError("Wrong end byte received.");
            }
            else {
                DebugInfo("Received end byte. Time passed: %.8f",
                    timer.getDeltaSeconds(false));
                waitingForEnd = false;
            }
        }
    }

    return 0;
}
