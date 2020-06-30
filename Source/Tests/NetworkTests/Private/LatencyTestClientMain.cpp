#include "SDL.h"
#include "SDL_net.h"
#include <iostream>
#include <string>
#include <array>
#include <cstdio>
#include "Timer.h"

//const std::string SERVER_IP = "127.0.0.1";
const std::string SERVER_IP = "45.79.37.63";
static constexpr unsigned int SERVER_PORT = 41499;
static constexpr unsigned int NUM_BYTES = 100;

using namespace AM;

int main(int argc, char* argv[])
{
    if (SDL_Init(0) == -1) {
        std::cout << "SDLNet_Init: " << SDLNet_GetError() << std::endl;
        return 1;
    }
    if (SDLNet_Init() == -1) {
        std::cout << "SDLNet_Init: " << SDLNet_GetError() << std::endl;
        return 2;
    }

    std::cout << "Connecting to server." << std::endl;

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, SERVER_IP.c_str(), SERVER_PORT)) {
        std::cout << "Could not resolve host." << std::endl;
        return 3;
    }

    TCPsocket socket = SDLNet_TCP_Open(&ip);
    if (!socket) {
        std::cout << "Could not open socket." << std::endl;
        return 4;
    }

    std::array<Uint8, 100> messageBuffer = {};
    Timer rttTimer;
    rttTimer.updateSavedTime();
    bool exitRequested = false;
    while (!exitRequested) {
        // Send
        int bytesSent = SDLNet_TCP_Send(socket, &messageBuffer, NUM_BYTES);
        if (bytesSent < NUM_BYTES) {
            std::cout << "Failed to send all bytes." << std::endl;
            return 5;
        }

        // Receive
        int result = SDLNet_TCP_Recv(socket, &messageBuffer, NUM_BYTES);
        if (result == NUM_BYTES) {
            float rtt = rttTimer.getDeltaSeconds(true);
            printf("Round trip time: %.6f seconds.", rtt);
            std::cout << std::endl;
        }
        else if (result <= 0) {
            // Disconnected
            std::cout << "Detected disconnect." << std::endl;
            return 7;
        }
        else {
            std::cout << "Didn't get all expected bytes."
            << std::endl;
            return 8;
        }
    }

    return 0;
}
