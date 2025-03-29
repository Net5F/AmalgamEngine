#pragma once

namespace AM
{
class EventDispatcher;
class CastableData;

namespace Client
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

    EventDispatcher& uiEventDispatcher;

    Network& network;

    GraphicData& graphicData;

    IconData& iconData;

    ItemData& itemData;

    CastableData& castableData;
};

} // namespace Client
} // namespace AM
