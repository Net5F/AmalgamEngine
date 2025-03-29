#pragma once

namespace AM
{
class CastableData;

namespace Server
{
class Simulation;
class Network;
class GraphicData;
class IconData;
class ItemData;

/**
 * Defines the dependencies that will be provided to the project's
 * SimulationExtension class.
 */
struct SimulationExDependencies {
public:
    Simulation& simulation;

    Network& network;

    GraphicData& graphicData;

    IconData& iconData;

    ItemData& itemData;

    CastableData& castableData;
};

} // namespace Server
} // namespace AM
