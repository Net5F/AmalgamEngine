#pragma once

namespace AM
{
class EventDispatcher;

namespace Client
{
class Network;
class SpriteData;

/**
 * Defines the dependencies that will be injected into the project's 
 * SimulationExtension class.
 */
struct SimulationExDependencies
{
public:
    EventDispatcher& inUiEventDispatcher;

    Network& inNetwork;

    SpriteData& inSpriteData;
};

} // namespace Client
} // namespace AM
