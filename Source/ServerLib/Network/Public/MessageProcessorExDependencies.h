#pragma once

namespace AM
{
class EventDispatcher;

namespace Server
{

/**
 * Defines the dependencies that will be injected into the project's 
 * MessageProcessorExtension class.
 */
struct MessageProcessorExDependencies
{
public:
    EventDispatcher& networkEventDispatcher;
};

} // namespace Server
} // namespace AM
