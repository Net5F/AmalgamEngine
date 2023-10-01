#pragma once

#include <SDL_stdinc.h>
#include <cstddef>

namespace AM
{
namespace Client
{

/**
 * Defines an extension for the engine's Client::MessageProcessor class.
 *
 * Extensions are implemented by the project, and are given generic functions
 * ("hooks") in which they can implement relevant project logic.
 *
 * The project can register the extension class with the engine through
 * Application::registerMessageProcessorExtension().
 */
class IMessageProcessorExtension
{
public:
    // Canonical constructor (derived class must implement):
    // MessageProcessorExtension(const MessageProcessorExDependencies& deps)

    /**
     * Called when a message is received that the engine doesn't have a handler
     * for.
     */
    virtual void processReceivedMessage(Uint8 messageType,
                                        Uint8* messageBuffer,
                                        std::size_t messageSize)
        = 0;
};

} // namespace Client
} // namespace AM
