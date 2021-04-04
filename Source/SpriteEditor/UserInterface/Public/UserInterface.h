#pragma once

#include "EventHandler.h"
#include "AUI/Screen.h"

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
     * The current active UI screen.
     */
    AUI::Screen currentScreen;

private:
    void handleMouseMotion(SDL_MouseMotionEvent& event);

    void handleMouseButtonDown(SDL_MouseButtonEvent& event);
};

} // namespace SpriteEditor
} // namespace AM
