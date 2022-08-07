#pragma once

namespace AM
{
class EventDispatcher;

namespace Client
{
class World;
class Network;
class SpriteData;

/**
 * Defines the dependencies that will be injected into the project's
 * SimulationExtension class.
 */
struct SimulationExDependencies {
public:
    World& world;

    EventDispatcher& uiEventDispatcher;

    Network& network;

    SpriteData& spriteData;
};

} // namespace Client
} // namespace AM
