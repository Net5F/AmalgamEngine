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
static constexpr unsigned int NUM_BYTES = 500;

using namespace AM;

int main(int argc, char* argv[])
{
    int iterationsToRun = 0;
    if (argc != 2) {
        std::cout << "Usage: ./LatencyTestClient <iterations>" << std::endl;
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
    std::cout << "Connected." << std::endl;

    // Build the string to send/receive.
    std::array<Uint8, NUM_BYTES> dataBuffer = {};
    for (unsigned int i = 0; i < NUM_BYTES; ++i) {
        dataBuffer[i] = (i % SDL_MAX_UINT8);
    }

    /* Send first. */
    std::cout << "Sending." << std::endl;
    Timer testTimer;
    std::vector<float> timeVec;
    for (int i = 0; i < iterationsToRun; ++i) {
        // Send
        testTimer.updateSavedTime();
        int bytesSent = SDLNet_TCP_Send(socket, &dataBuffer, NUM_BYTES);
        timeVec.push_back(testTimer.getDeltaSeconds(false));
        if (bytesSent < NUM_BYTES) {
            std::cout << "Failed to send all bytes." << std::endl;
            return 5;
        }
    }
    for (auto time : timeVec) {
        printf("Time: %.6f KB/s", time);
        std::cout << std::endl;
    }

    /* Wait for the other side to signal it's done receiving. */
    std::cout << "Done sending. Waiting for done byte." << std::endl;
    std::array<Uint8, NUM_BYTES> recBuffer = {};
    int result = SDLNet_TCP_Recv(socket, &(recBuffer[0]), 1);
    if (result <= 0) {
        // Disconnected
        std::cout << "Detected disconnect." << std::endl;
        return 7;
    }
    else if (result > 1) {
        std::cout << "Received more than just the done byte." << std::endl;
        return 7;
    }
    else {
        std::cout << "Done byte received." << std::endl;
    }
//
//    Timer testTimer;
//
//    /* Receive. */
//    std::array<Uint8, NUM_BYTES> recBuffer = {};
//    std::cout << "Done sending. Waiting to receive:" << std::endl;
//
//    // TODO: After this is solved, add per-message timing and average/min/max on each side.
//    bool receiveStarted = false;
//    for (int i = 0; i < iterationsToRun; ++i) {
//        // Receive
//        int receivedBytes = 0;
//        while (receivedBytes < NUM_BYTES) {
//            int result = SDLNet_TCP_Recv(socket, &(recBuffer[receivedBytes]),
//                (NUM_BYTES - receivedBytes));
//            if (result <= 0) {
//                // Disconnected
//                std::cout << "Detected disconnect." << std::endl;
//                return 7;
//            }
//
//            receivedBytes += result;
//
//            if (!receiveStarted) {
//                // If we received for the first time, start the timer.
//                testTimer.updateSavedTime();
//                receiveStarted = true;
//            }
//        }
//
//        // Check the integrity of the received data.
//        if (receivedBytes == NUM_BYTES) {
//            for (unsigned int i = 0; i < NUM_BYTES; ++i) {
//                if (recBuffer[i] != dataBuffer[i]) {
//                    std::cout << "Received data out of order. Rec : "
//                    << std::to_string(recBuffer[i]) << " expected: "
//                    << std::to_string(dataBuffer[i]) << std::endl;
//                }
//            }
//        }
//        else if (receivedBytes > NUM_BYTES){
//            std::cout << "Received too many bytes. Something is broken." << std::endl;
//            return 8;
//        }
//    }
//    float testTime = testTimer.getDeltaSeconds(true);
//
//    /* Done getting data. Display it. */
//    float totalBytes = static_cast<float>(iterationsToRun - 1)
//    * static_cast<float>(NUM_BYTES);
//
//    // Throughput in KB/S
//    float throughput = (totalBytes / 1000) / testTime;
//
//    std::cout << "## Throughput calcs ##" << std::endl;
//    printf("%f bytes in %f seconds.", totalBytes, testTime);
//    std::cout << std::endl;
//    printf("Throughput: %.6f KB/s", throughput);
//    std::cout << std::endl;

    return 0;
}
