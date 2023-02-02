#pragma once

namespace AM
{
class EventDispatcher;

namespace Server
{

/**
 * Defines the dependencies that will be provided to the project's
 * MessageProcessorExtension class.
 */
struct MessageProcessorExDependencies {
public:
    EventDispatcher& networkEventDispatcher;
};

} // namespace Server
} // namespace AM
