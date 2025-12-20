#pragma once

#include "entt/signal/fwd.hpp"

struct SDL_Renderer;

namespace AM
{
class EventDispatcher;

namespace Client
{
class Simulation;
class WorldObjectLocator;
class Network;
class GraphicData;
class ItemData;
class IconData;

/**
 * Provides the dependencies that UserInterface objects may use.
 */
struct UserInterfaceContext {
public:
    /** World state, current tick. */
    Simulation& simulation;

    /** Find entities or tile layers that a mouse event hit. */
    const WorldObjectLocator& worldObjectLocator;

    /** Server messaging. */
    Network& network;

    /** UI -> Sim events. */
    entt::dispatcher& uiEventDispatcher;

    /** Sim -> UI events. */
    entt::dispatcher& simEventDispatcher;

    /** Network -> Sim/UI message events. */
    EventDispatcher& networkEventDispatcher;

    SDL_Renderer* sdlRenderer;

    GraphicData& graphicData;

    IconData& iconData;

    ItemData& itemData;
};

} // namespace Client
} // namespace AM
