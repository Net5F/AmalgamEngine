#pragma once

#include "EventHandler.h"
#include "TitleScreen.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"

#include "AUI/Initializer.h"

// Forward declarations.
struct SDL_Renderer;

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
/**
 * This class handles creation and management of the user interface.
 *
 * Additionally, it provides a way for the renderer to access the UI data.
 *
 * Note: This UI setup is currently an exploration in not using signals.
 *       Eventually, this will be updated to use signals for better separation
 *       between the UI and back-end logic.
 */
class UserInterface : public EventHandler
{
public:
    UserInterface(SDL_Renderer* renderer, AssetCache& assetCache,
                  SpriteDataModel& spriteDataModel);

    /**
     * Changes the currentScreen to titleScreen.
     */
    void openTitleScreen();

    /**
     * Tells mainScreen to load the current sprite data into its UI, and changes
     * the currentScreen to mainScreen.
     */
    void openMainScreen();

    /**
     * Handles user input events.
     */
    bool handleEvent(SDL_Event& event) override;

    /**
     * Calls AUI::Screen::tick() on the current screen.
     *
     * @param timestepS  The amount of time that has passed since the last
     *                   tick() call, in seconds.
     */
    void tick(double timestepS);

    /**
     * AmalgamUI initializer, used to init/quit the library at the proper
     * times.
     */
    AUI::Initializer auiInitializer;

    /**
     * The current active UI screen.
     */
    AUI::Screen* currentScreen;

private:
    TitleScreen titleScreen;

    MainScreen mainScreen;
};

} // namespace SpriteEditor
} // namespace AM
