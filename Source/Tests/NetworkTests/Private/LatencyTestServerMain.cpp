#include "SDL.h"
#include "SDL_net.h"
#include <iostream>
#include <array>

static constexpr int SERVER_PORT = 41499;
static constexpr unsigned int NUM_BYTES = 100;

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
    std::array<Uint8, 100> messageBuffer = {};
    std::cout << "Server started." << std::endl;
    bool exitRequested = false;
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
                    SDLNet_TCP_DelSocket(clientSet, clientSocket);
                    SDLNet_TCP_Close(clientSocket);
                    std::cout << "Failed to send all bytes. Cleaned up connection."
                    << std::endl;
                }
            }
            else if (result <= 0) {
                // Disconnected
                SDLNet_TCP_DelSocket(clientSet, clientSocket);
                SDLNet_TCP_Close(clientSocket);
                std::cout << "Detected disconnect. Cleaned up connection." << std::endl;
            }
            else {
                SDLNet_TCP_DelSocket(clientSet, clientSocket);
                SDLNet_TCP_Close(clientSocket);
                std::cout << "Didn't get all expected bytes. Cleaned up connection."
                << std::endl;
            }
        }
    }

    return 0;
}
