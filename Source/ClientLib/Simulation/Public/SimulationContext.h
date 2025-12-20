#pragma once

#include "entt/signal/fwd.hpp"

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
 * Provides the dependencies that the project's Simulation objects may use.
 */
struct SimulationContext {
public:
    /** World state, current tick. */
    Simulation& simulation;

    /** Server messaging. */
    Network& network;

    /** Sim -> UI events. */
    entt::dispatcher& simEventDispatcher;

    /** UI -> Sim events. */
    entt::dispatcher& uiEventDispatcher;

    /** Network -> Sim/UI message events. */
    EventDispatcher& networkEventDispatcher;

    GraphicData& graphicData;

    IconData& iconData;

    ItemData& itemData;

    CastableData& castableData;
};

} // namespace Client
} // namespace AM
