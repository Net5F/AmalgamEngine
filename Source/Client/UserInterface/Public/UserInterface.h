#pragma once

#include "OSEventHandler.h"
#include "Sprite.h"
#include "ScreenPoint.h"
#include "TileIndex.h"
#include "SDL2pp/Texture.hh"
#include <memory>
#include <vector>

namespace AM
{
class EventDispatcher;

namespace Client
{
class World;
class SpriteData;

/**
 * Uses user input and sim data to manage the state of the user interface.
 */
class UserInterface : public OSEventHandler
{
public:
    UserInterface(EventDispatcher& inUiEventDispatcher, const World& inWorld,
                  SpriteData& spriteData);

    /**
     * Handles user input events.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /** Sprite for the mouse-following tile highlight. */
    const Sprite* tileHighlightSprite;

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

    /** Used to send UI events to the subscribed Simulation systems. */
    EventDispatcher& uiEventDispatcher;

    /** Used to get the world state to populate the UI. */
    const World& world;

    /** Holds the sprites that we cycle through on mouse click. */
    std::vector<const Sprite*> terrainSprites;
};

} // End namespace Client
} // End namespace AM
