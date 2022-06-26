#pragma once

#include "OSEventHandler.h"
#include "PeriodicCaller.h"

struct SDL_Renderer;

namespace AM
{
namespace SpriteEditor
{
class UserInterface;

/**
 * Renders the UI.
 */
class Renderer : public OSEventHandler
{
public:
    static constexpr unsigned int FRAMES_PER_SECOND = 60;
    static constexpr double FRAME_TIMESTEP_S
        = 1.0 / static_cast<double>(FRAMES_PER_SECOND);

    Renderer(SDL_Renderer* inSdlRenderer, UserInterface& inUI);

    /**
     * First renders all tiles in view, then renders all entities in view.
     */
    void render();

    /**
     * Handles window events.
     */
    bool handleOSEvent(SDL_Event& event) override;

private:
    SDL_Renderer* sdlRenderer;

    /** Used to begin the UI rendering. */
    UserInterface& ui;
};

} // namespace SpriteEditor
} // namespace AM
