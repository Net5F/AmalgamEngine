#pragma once

struct SDL_Renderer;

namespace AM
{
class EventDispatcher;
class AssetCache;

namespace Client
{
class WorldSignals;
class SpriteData;

/**
 * Defines the dependencies that will be provided to the project's
 * UserInterfaceExtension class.
 */
struct UserInterfaceExDependencies {
public:
    WorldSignals& worldSignals;

    EventDispatcher& uiEventDispatcher;

    SDL_Renderer* sdlRenderer;

    AssetCache& assetCache;

    SpriteData& spriteData;
};

} // namespace Client
} // namespace AM
