#include "SDL.h"
#include "SDL_net.h"
#include <iostream>
#include <array>
#include <atomic>
#include <thread>
#include "Timer.h"

static constexpr int SERVER_PORT = 41499;
static constexpr unsigned int NUM_BYTES = 500;

using namespace AM;

int main(int argc, char* argv[])
{
    int iterationsToRun = 0;
    if (argc != 2) {
        std::cout << "Usage: ./LatencyTestServer <iterations>" << std::endl;
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

    /* Set up the listener. */
    IPaddress ip;
    TCPsocket serverSocket = nullptr;
    if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT)) {
        std::cerr << SDLNet_GetError() << std::endl;
    }

    serverSocket = SDLNet_TCP_Open(&ip);
    if (!serverSocket) {
        std::cerr << SDLNet_GetError() << std::endl;
    }

    TCPsocket clientSocket = nullptr;
    SDLNet_SocketSet clientSet = SDLNet_AllocSocketSet(1);

    std::cout << "Server started." << std::endl;

    // Wait for a connection.
    while (clientSocket == nullptr) {
        SDL_Delay(1);
        clientSocket = SDLNet_TCP_Accept(serverSocket);

        // If we got a connection, add the socket to the set.
        if (clientSocket != nullptr) {
            int numAdded = SDLNet_TCP_AddSocket(clientSet, clientSocket);
            if (numAdded < 1) {
                std::cout << "Error while adding socket: " << SDLNet_GetError()
                << std::endl;
            }
            else {
                std::cout << "Connected new client." << std::endl;
            }
        }
    }

    // Build the string to send/receive.
    std::array<Uint8, NUM_BYTES> dataBuffer = {};
    for (unsigned int i = 0; i < NUM_BYTES; ++i) {
        dataBuffer[i] = (i % SDL_MAX_UINT8);
    }

    Timer testTimer;

    /* Receive first. */
    std::array<Uint8, NUM_BYTES> recBuffer = {};
    testTimer.updateSavedTime();
    bool receiveStarted = false;
    for (int i = 0; i < iterationsToRun; ++i) {
        // Receive
        int receivedBytes = 0;
        while (receivedBytes < NUM_BYTES) {
            std::cout << "In" << std::endl;
            int result = SDLNet_TCP_Recv(clientSocket, &(recBuffer[receivedBytes]),
                (NUM_BYTES - receivedBytes));
            std::cout << "result: " << result << std::endl;
            if (result <= 0) {
                // Disconnected
                perror("Detected disconnect");
                std::cout << "result: " << result << std::endl;
                std::cout << "receivedBytes: " << receivedBytes << std::endl;
                std::cout << "i: " << i << std::endl;
                SDLNet_TCP_DelSocket(clientSet, clientSocket);
                SDLNet_TCP_Close(clientSocket);
                clientSocket = nullptr;
                return 7;
            }

            receivedBytes += result;

            if (!receiveStarted) {
                // If we received for the first time, start the timer.
                testTimer.updateSavedTime();
                receiveStarted = true;
            }
        }
        std::cout << "Stop. i: " << i << std::endl;

        // Check the integrity of the received data.
        if (receivedBytes == NUM_BYTES) {
            for (unsigned int i = 0; i < NUM_BYTES; ++i) {
                if (recBuffer[i] != dataBuffer[i]) {
                    std::cout << "Received data out of order. Rec : "
                    << std::to_string(recBuffer[i]) << " expected: "
                    << std::to_string(dataBuffer[i]) << std::endl;
                }
            }
        }
        else if (receivedBytes > NUM_BYTES){
            std::cout << "Received too many bytes. Something is broken." << std::endl;
            SDLNet_TCP_DelSocket(clientSet, clientSocket);
            SDLNet_TCP_Close(clientSocket);
            clientSocket = nullptr;
            return 8;
        }
    }
    double testTime = testTimer.getDeltaSeconds(true);

    /* Done getting data. Display it. */
    float totalBytes = static_cast<float>(iterationsToRun - 1)
    * static_cast<float>(NUM_BYTES);

    // Throughput in KB/S
    float throughput = (totalBytes / 1000) / testTime;

    std::cout << "## Throughput calcs ##" << std::endl;
    printf("%f bytes in %f seconds.", totalBytes, testTime);
    std::cout << std::endl;
    printf("Throughput: %.6f KB/s", throughput);
    std::cout << std::endl;

    /* Done displaying. Send the done byte. */
    unsigned int doneByte = 42;
    int bytesSent = SDLNet_TCP_Send(clientSocket, &doneByte, 1);
    if (bytesSent < 1) {
        std::cout << "Failed to send done byte." << std::endl;
        SDLNet_TCP_DelSocket(clientSet, clientSocket);
        SDLNet_TCP_Close(clientSocket);
        clientSocket = nullptr;
    }
    // Wait to make sure the done byte is separately received.
    SDL_Delay(100);

    /* Send. */
//    for (int i = 0; i < iterationsToRun; ++i) {
//        int bytesSent = SDLNet_TCP_Send(clientSocket, &dataBuffer, NUM_BYTES);
//        if (bytesSent < NUM_BYTES) {
//            std::cout << "Failed to send all bytes." << std::endl;
//            SDLNet_TCP_DelSocket(clientSet, clientSocket);
//            SDLNet_TCP_Close(clientSocket);
//            clientSocket = nullptr;
//        }
//    }
//    std::cout << "Done sending." << std::endl;

    return 0;
}
