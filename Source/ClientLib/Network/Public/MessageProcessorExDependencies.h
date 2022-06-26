#pragma once

namespace AM
{
class EventDispatcher;

namespace Client
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

} // namespace Client
} // namespace AM
