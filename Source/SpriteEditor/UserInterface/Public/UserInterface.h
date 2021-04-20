#pragma once

#include "EventHandler.h"
#include "TitleScreen.h"
#include "AUI/Initializer.h"

// Forward declarations.
struct SDL_Renderer;

namespace AM
{
namespace SpriteEditor
{

/**
 * This class handles creation and management of the user interface.
 *
 * Additionally, it provides a way for the renderer to access the UI data.
 */
class UserInterface : public EventHandler
{
public:
    UserInterface(SDL_Renderer* renderer);

    /**
     * Handles user input events.
     */
    bool handleEvent(SDL_Event& event) override;

    /**
     * AmalgamUI initializer, used to init/quit the library at the proper
     * times.
     */
    AUI::Initializer initializer;

    /**
     * The current active UI screen.
     */
    AUI::Screen* currentScreen;

private:
    TitleScreen titleScreen;
};

} // namespace SpriteEditor
} // namespace AM
