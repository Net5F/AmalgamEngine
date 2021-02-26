#pragma once

#include "EventHandler.h"

namespace AM
{
namespace Client
{

class World;

/**
 * Uses user input and sim data to manage the state of the user interface.
 */
class UserInterface : public EventHandler
{
public:
    UserInterface(World& inWorld);

    /**
     * Handles user input events.
     */
    bool handleEvent(SDL_Event& event) override;

private:
    void handleMouseMotion(SDL_MouseMotionEvent& event);

    World& world;

    // Highlight sprite and position
};

} // End namespace Client
} // End namespace AM
