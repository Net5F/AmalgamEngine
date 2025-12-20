#pragma once

namespace AM
{
class EventDispatcher;

namespace Client
{

/**
 * Provides the dependencies that MessageProcessor logic may use.
 */
struct MessageProcessorContext {
public:
    EventDispatcher& networkEventDispatcher;
};

} // namespace Client
} // namespace AM
