#pragma once

#include "EventHandler.h"
#include "ResourceManager.h"

namespace AM
{
namespace SpriteEditor
{

/**
 *
 */
class UserInterface : public EventHandler
{
public:
    UserInterface(ResourceManager& inResourceManager);

    /**
     * Handles user input events.
     */
    bool handleEvent(SDL_Event& event) override;

private:
    void handleMouseMotion(SDL_MouseMotionEvent& event);

    void handleMouseButtonDown(SDL_MouseButtonEvent& event);

    ResourceManager& resourceManager;
};

} // End namespace SpriteEditor
} // End namespace AM
