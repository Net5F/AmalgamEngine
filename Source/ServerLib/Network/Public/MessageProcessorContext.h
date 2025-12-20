#pragma once

namespace AM
{
class EventDispatcher;

namespace Server
{

/**
 * Provides the dependencies that MessageProcessor logic may use.
 */
struct MessageProcessorContext {
public:
    EventDispatcher& networkEventDispatcher;
};

} // namespace Server
} // namespace AM
