#pragma once

#include "EventHandler.h"
#include "Sprite.h"
#include "ScreenPoint.h"
#include "TileIndex.h"
#include "SDL2pp/Texture.hh"
#include <memory>
#include <vector>

namespace AM
{
class AssetCache;

namespace Client
{
class World;

/**
 * Uses user input and sim data to manage the state of the user interface.
 */
class UserInterface : public EventHandler
{
public:
    UserInterface(World& inWorld, AssetCache& inAssetCache);

    /**
     * Handles user input events.
     */
    bool handleEvent(SDL_Event& event) override;

    /** Sprite for the mouse-following tile highlight. */
    Sprite tileHighlightSprite;

    /** Index of the tile to be highlighted. */
    TileIndex tileHighlightIndex;

private:
    void handleMouseMotion(SDL_MouseMotionEvent& event);

    void handleMouseButtonDown(SDL_MouseButtonEvent& event);

    /**
     * Cycles the tile under the given mouse position to the next sprite in
     * tileHighlightSprites.
     */
    void cycleTile(int mouseX, int mouseY);

    /** Used to get the world state to populate the UI. */
    World& world;

    /** Used for loading UI textures. */
    AssetCache& assetCache;

    /** Holds the sprites that we cycle through on mouse click. */
    std::vector<Sprite> terrainSprites;
};

} // End namespace Client
} // End namespace AM
