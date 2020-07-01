#include "SDL.h"
#include "SDL_net.h"
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cstdio>
#include "Timer.h"

//const std::string SERVER_IP = "127.0.0.1";
const std::string SERVER_IP = "45.79.37.63";
static constexpr unsigned int SERVER_PORT = 41499;
static constexpr unsigned int NUM_BYTES = 64;

using namespace AM;

int main(int argc, char* argv[])
{
    int iterationsToRun = 0;
    if (argc != 2) {
        std::cout << "Usage: ./LatencyTestClient <number>" << std::endl;
        return 0;
    }
    else {
        iterationsToRun = std::stoi(argv[1]);
    }

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

    int iterationCount = 0;

    std::array<Uint8, NUM_BYTES> messageBuffer = {};

    std::cout << "Running tests" << std::endl;
    Timer testTimer;
    testTimer.updateSavedTime();
    while (iterationCount < iterationsToRun) {
        // Send
        int bytesSent = SDLNet_TCP_Send(socket, &messageBuffer, NUM_BYTES);
        if (bytesSent < NUM_BYTES) {
            std::cout << "Failed to send all bytes." << std::endl;
            return 5;
        }

        // Receive
        int result = SDLNet_TCP_Recv(socket, &messageBuffer, NUM_BYTES);
        if (result == NUM_BYTES) {
            iterationCount++;
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
    float testTime = testTimer.getDeltaSeconds(true);

    /* Done getting data. Display it. */
    float bytesLooped = static_cast<float>(iterationsToRun) * static_cast<float>(NUM_BYTES);

    // Throughput in KB/S
    float throughput = (bytesLooped / 1000) / testTime;

    std::cout << "## Throughput calcs ##" << std::endl;
    printf("Throughput: %.6f KB/s", throughput);
    std::cout << std::endl;

    return 0;
}
