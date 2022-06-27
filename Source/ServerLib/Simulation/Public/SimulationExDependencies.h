#pragma once

namespace AM
{
namespace Server
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
    Network& inNetwork;

    SpriteData& inSpriteData;
};

} // namespace Server
} // namespace AM
