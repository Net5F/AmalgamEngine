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

/**
 * Defines the dependencies that will be provided to the project's
 * UserInterfaceExtension class.
 */
struct UserInterfaceExDependencies {
public:
    /** Const, only allows us to view world data. */
    const World& world;

    /** Non-const, used for subscribing to world data updates. */
    WorldSignals& worldSignals;

    /** Used for finding entities or tile layers that a mouse event hit. */
    const WorldObjectLocator& worldObjectLocator;

    /** Used to send events to the sim. */
    EventDispatcher& uiEventDispatcher;

    /** Used for rendering. */
    SDL_Renderer* sdlRenderer;

    /** Used for getting sprite and sprite set data. */
    SpriteData& spriteData;
};

} // namespace Client
} // namespace AM
