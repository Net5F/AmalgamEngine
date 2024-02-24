#pragma once

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
class IconData;

/**
 * Defines the dependencies that will be provided to the project's
 * UserInterfaceExtension class.
 */
struct UserInterfaceExDependencies {
public:
    /** Used for viewing world data and subscribing to signals. */
    Simulation& simulation;

    /** Used for finding entities or tile layers that a mouse event hit. */
    const WorldObjectLocator& worldObjectLocator;

    /** Used to send events to the sim. */
    EventDispatcher& uiEventDispatcher;

    /** Used to send and receive messages from the server. */
    Network& network;

    /** Used for rendering. */
    SDL_Renderer* sdlRenderer;

    /** Used for getting sprite and animation data. */
    GraphicData& graphicData;

    /** Used for getting icon data. */
    IconData& iconData;
};

} // namespace Client
} // namespace AM
