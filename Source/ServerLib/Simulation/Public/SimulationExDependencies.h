#pragma once

namespace AM
{
namespace Server
{
class Simulation;
class Network;
class GraphicData;

/**
 * Defines the dependencies that will be provided to the project's
 * SimulationExtension class.
 */
struct SimulationExDependencies {
public:
    Simulation& simulation;

    Network& network;

    GraphicData& graphicData;
};

} // namespace Server
} // namespace AM
