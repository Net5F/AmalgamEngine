#pragma once

#include "OSEventHandler.h"
#include "TitleScreen.h"
#include "MainScreen.h"
#include "DataModel.h"

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
class UserInterface : public OSEventHandler
{
public:
    enum class ScreenType { TitleScreen, MainScreen };

    UserInterface(SDL_Renderer* inRenderer, AssetCache& inAssetCache,
                  DataModel& inDataModel);

    /**
     * Changes the currentScreen to the given screenType.
     */
    void changeScreenTo(ScreenType screenType);

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
     */
    void render();

    /**
     * Handles user input events.
     */
    bool handleOSEvent(SDL_Event& event) override;

private:
    /** AmalgamUI initializer, used to init/quit the library at the proper
        times. */
    AUI::Initializer auiInitializer;

    /** The opening title screen, seen on app launch. */
    TitleScreen titleScreen;

    /** The main screen for doing work. */
    MainScreen mainScreen;

    /** The current active UI screen. */
    AUI::Screen* currentScreen;
};

} // namespace SpriteEditor
} // namespace AM
