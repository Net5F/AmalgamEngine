#include "SDL.h"
#include "SDL_net.h"
#include <iostream>
#include <array>
#include <atomic>
#include <thread>

static constexpr int SERVER_PORT = 41499;
static constexpr unsigned int NUM_BYTES = 64;

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
    std::array<Uint8, NUM_BYTES> messageBuffer = {};

    // Spin up a thread to check for command line input.
    std::atomic<bool> exitRequested = false;
    std::thread inputThreadObj(inputThread, &exitRequested);

    std::cout << "Server started." << std::endl;
    while (!exitRequested) {
        // If we don't have a connection, try to get one.
        if (clientSocket == nullptr) {
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
        else {
            // Wait for a message.
            int result = SDLNet_TCP_Recv(clientSocket, &messageBuffer, NUM_BYTES);
            if (result == NUM_BYTES) {
                // Got a message, loop it back.
                int bytesSent = SDLNet_TCP_Send(clientSocket, &messageBuffer, NUM_BYTES);
                if (bytesSent < NUM_BYTES) {
                    std::cout << "Failed to send all bytes. Cleaning up connection."
                    << std::endl;
                    SDLNet_TCP_DelSocket(clientSet, clientSocket);
                    SDLNet_TCP_Close(clientSocket);
                    clientSocket = nullptr;
                }
            }
            else if (result <= 0) {
                // Disconnected
                std::cout << "Detected disconnect. Cleaning up connection." << std::endl;
                SDLNet_TCP_DelSocket(clientSet, clientSocket);
                SDLNet_TCP_Close(clientSocket);
                clientSocket = nullptr;
            }
            else {
                std::cout << "Didn't get all expected bytes. Cleaning up connection."
                << std::endl;
                SDLNet_TCP_DelSocket(clientSet, clientSocket);
                SDLNet_TCP_Close(clientSocket);
                clientSocket = nullptr;
            }
        }
    }

    inputThreadObj.join();

    return 0;
}
