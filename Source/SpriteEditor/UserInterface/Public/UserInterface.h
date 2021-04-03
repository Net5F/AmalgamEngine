#pragma once

#include "EventHandler.h"
#include "ResourceManager.h"
#include "AUI/Page.h"

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
    UserInterface(ResourceManager& inResourceManager);

    /**
     * Handles user input events.
     */
    bool handleEvent(SDL_Event& event) override;

    AUI::Page currentPage;

private:
    void handleMouseMotion(SDL_MouseMotionEvent& event);

    void handleMouseButtonDown(SDL_MouseButtonEvent& event);

    ResourceManager& resourceManager;
};

} // namespace SpriteEditor
} // namespace AM
