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

/**
 * Defines the dependencies that will be provided to the project's
 * UserInterfaceExtension class.
 */
struct UserInterfaceExDependencies {
public:
    /** Const, only allows us to view world data. */
    const World& world;

    /** Non-const, allows us to subscribe to world data updates. */
    WorldSignals& worldSignals;

    EventDispatcher& uiEventDispatcher;

    SDL_Renderer* sdlRenderer;

    SpriteData& spriteData;
};

} // namespace Client
} // namespace AM
