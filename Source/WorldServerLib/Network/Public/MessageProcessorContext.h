#pragma once

namespace AM
{
class EventDispatcher;

namespace WorldServer
{

/**
 * Provides the dependencies that MessageProcessor logic may use.
 */
struct MessageProcessorContext {
public:
    EventDispatcher& networkEventDispatcher;
};

} // namespace WorldServer
} // namespace AM
