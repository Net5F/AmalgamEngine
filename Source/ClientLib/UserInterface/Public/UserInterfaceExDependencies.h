#pragma once

struct SDL_Renderer;

namespace AM
{
class EventDispatcher;

namespace Client
{
class World;
class WorldSignals;
class SpriteData;
class WorldObjectLocator;
class Network;

/**
 * Defines the dependencies that will be provided to the project's
 * UserInterfaceExtension class.
 */
struct UserInterfaceExDependencies {
public:
    /** Used for viewing world data and subscribing to entity registry
        updates. */
    World& world;

    /** Used for subscribing to non-registry world state updates. */
    WorldSignals& worldSignals;

    /** Used for finding entities or tile layers that a mouse event hit. */
    const WorldObjectLocator& worldObjectLocator;

    /** Used to send events to the sim. */
    EventDispatcher& uiEventDispatcher;

    /** Used to send and receive messages from the server. */
    Network& network;

    /** Used for rendering. */
    SDL_Renderer* sdlRenderer;

    /** Used for getting sprite and sprite set data. */
    SpriteData& spriteData;
};

} // namespace Client
} // namespace AM
