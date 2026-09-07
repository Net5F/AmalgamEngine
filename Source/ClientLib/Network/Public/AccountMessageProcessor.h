#pragma once

#include "AccountClientMessageType.h"
#include "asio/error_code.hpp"
#include <SDL3/SDL_stdinc.h>
#include <functional>
#include <span>

namespace AM
{
class EventDispatcher;

namespace Client
{
struct MessageProcessorContext;

/**
 * Processes AccountServer messages and dispatches them to the simulation.
 */
class AccountMessageProcessor
{
public:
    using DisconnectCallback = std::function<void(const asio::error_code&)>;

    AccountMessageProcessor(
        const MessageProcessorContext& inMessageProcessorContext,
        DisconnectCallback inDisconnectCallback);

    /**
     * Deserializes and dispatches a received AccountServer message.
     */
    void processReceivedMessage(AccountClientMessageType messageType,
                                std::span<const Uint8> messageBuffer);

private:
    template<typename Message>
    void dispatchMessage(std::span<const Uint8> messageBuffer);

    EventDispatcher& networkEventDispatcher;

    /** Used to disconnect from an AccountServer that sends invalid messages. */
    DisconnectCallback disconnectCallback;
};

} // namespace Client
} // namespace AM
