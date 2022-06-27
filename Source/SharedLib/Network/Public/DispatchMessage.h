#pragma once

#include "Deserialize.h"
#include "QueuedEvents.h"
#include <SDL_stdinc.h>
#include <memory>

namespace AM
{

/**
 * Deserializes the message in the given buffer and pushes it through the
 * given event dispatcher.
 * Used when the message type doesn't require any network-level processing.
 *
 * The event can be received in a system using EventQueue<T>.
 */
template<typename T>
static void dispatchMessage(Uint8* messageBuffer, unsigned int messageSize,
                            EventDispatcher& dispatcher)
{
    // Deserialize the message.
    T message{};
    Deserialize::fromBuffer(messageBuffer, messageSize, message);

    // Push the message into any subscribed queues.
    dispatcher.push<T>(message);
}

/**
 * Similar to pushEvent(), but allocates the event to the heap through a
 * std::shared_ptr.
 *
 * Used for large events or events with internal allocations, where the
 * cost of allocating once is lower than copying.
 *
 * The event can be received in a system using
 * EventQueue<std::shared_ptr<const T>>.
 */
template<typename T>
static void dispatchMessageSharedPtr(Uint8* messageBuffer,
                                     unsigned int messageSize,
                                     EventDispatcher& dispatcher)
{
    // Deserialize the message.
    std::shared_ptr<T> message{std::make_shared<T>()};
    Deserialize::fromBuffer(messageBuffer, messageSize, *message);

    // Push the message into any subscribed queues.
    dispatcher.push<std::shared_ptr<const T>>(message);
}

} // End namespace AM
