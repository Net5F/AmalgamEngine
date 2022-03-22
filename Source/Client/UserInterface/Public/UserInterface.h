#pragma once

#include "OSEventHandler.h"
#include "Sprite.h"
#include "ScreenPoint.h"
#include "TilePosition.h"
#include "AUI/Initializer.h"
#include "SDL2pp/Texture.hh"
#include <memory>
#include <vector>

// Forward declarations.
struct SDL_Renderer;

namespace AUI
{
class Screen;
}

namespace AM
{
class EventDispatcher;
class AssetCache;
class Camera;

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
                  SDL_Renderer* inSDLRenderer, AssetCache& inAssetCache
                  , SpriteData& inSpriteData);

    /**
     * Handles user input events.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * Renders all UI graphics for the current screen to the current rendering
     * target.
     *
     * @param camera  The camera to calculate screen position with.
     */
    void render(const Camera& camera);

private:
    void handleMouseMotion(SDL_MouseMotionEvent& event);

    void handleMouseButtonDown(SDL_MouseButtonEvent& event);

    /**
     * Cycles the tile under the given mouse position to the next sprite in
     * terrainSprites.
     */
    void cycleTile(int mouseX, int mouseY);

    /** Used to send UI events to the subscribed Simulation systems. */
    EventDispatcher& uiEventDispatcher;

    /** Used to get the world state to populate the UI. */
    const World& world;

    // TODO: Remove this member when we stop rendering in this class
    /** Used to render UI graphics. */
    SDL_Renderer* sdlRenderer;

    // TODO: Remove this member when we start passing it to a screen
    /** Used to get UI textures. */
    AssetCache& assetCache;

    /**
     * AmalgamUI initializer, used to init/quit the library at the proper
     * times.
     */
    AUI::Initializer auiInitializer;

    /**
     * The current active UI screen.
     */
    AUI::Screen* currentScreen;

    /** Holds the sprites that we cycle through on mouse click. */
    std::vector<const Sprite*> terrainSprites;

    /** Sprite for the mouse-following tile highlight. */
    const Sprite* tileHighlightSprite;

    /** Position of the tile to be highlighted. */
    TilePosition tileHighlightPosition;
};

} // End namespace Client
} // End namespace AM
