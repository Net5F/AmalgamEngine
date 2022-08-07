#pragma once

namespace AM
{
namespace Server
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

    Network& network;

    SpriteData& spriteData;
};

} // namespace Server
} // namespace AM
