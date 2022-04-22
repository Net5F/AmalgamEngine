#pragma once

#include "OSEventHandler.h"
#include "MainScreen.h"
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
struct Camera;

namespace Client
{
class WorldSinks;
class SpriteData;

/**
 * Uses user input and sim data to manage the state of the user interface.
 */
class UserInterface : public OSEventHandler
{
public:
    UserInterface(WorldSinks& inWorldSinks,
                  EventDispatcher& inUiEventDispatcher,
                  SDL_Renderer* inSDLRenderer, AssetCache& inAssetCache,
                  SpriteData& inSpriteData);

    /**
     * Handles user input events.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * Calls AUI::Screen::tick() on the current screen.
     *
     * @param timestepS  The amount of time that has passed since the last
     *                   tick() call, in seconds.
     */
    void tick(double timestepS);

    /**
     * Renders all UI graphics for the current screen to the current rendering
     * target.
     *
     * @param camera  The camera to calculate screen position with.
     */
    void render(const Camera& camera);

private:
    /**
     * Cycles the tile under the given mouse position to the next sprite in
     * terrainSprites.
     */
    void cycleTile(int mouseX, int mouseY);

    /** AmalgamUI initializer, used to init/quit the library at the proper
        times. */
    AUI::Initializer auiInitializer;

    /** The main UI that overlays the world. */
    MainScreen mainScreen;

    /** The current active UI screen. */
    AUI::Screen* currentScreen;
};

} // End namespace Client
} // End namespace AM
