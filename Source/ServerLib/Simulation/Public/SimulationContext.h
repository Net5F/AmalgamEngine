#pragma once

namespace AM
{
class EventDispatcher;
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
struct SimulationContext {
public:
    /** World state, current tick. */
    Simulation& simulation;

    /** Client messaging. */
    Network& network;

    /** Network -> Sim/UI message events. */
    EventDispatcher& networkEventDispatcher;

    GraphicData& graphicData;

    IconData& iconData;

    ItemData& itemData;

    CastableData& castableData;
};

} // namespace Server
} // namespace AM
