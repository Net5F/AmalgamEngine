#pragma once

namespace AM
{
class EventDispatcher;

namespace Client
{
class Simulation;
class Network;
class SpriteData;

/**
 * Defines the dependencies that will be provided to the project's
 * SimulationExtension class.
 */
struct SimulationExDependencies {
public:
    Simulation& simulation;

    EventDispatcher& uiEventDispatcher;

    Network& network;

    SpriteData& spriteData;
};

} // namespace Client
} // namespace AM
