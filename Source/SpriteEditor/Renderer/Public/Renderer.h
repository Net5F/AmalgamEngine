#pragma once

#include "EventHandler.h"
#include "PeriodicCaller.h"

#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"
#include "SDL2pp/Rect.hh"

namespace AM
{
namespace SpriteEditor
{
class UserInterface;

/**
 *
 */
class Renderer : public EventHandler
{
public:
    static constexpr unsigned int FRAMES_PER_SECOND = 60;
    static constexpr double FRAME_TIMESTEP_S
        = 1.0 / static_cast<double>(FRAMES_PER_SECOND);

    /**
     * @param getProgress  A function that returns how far between sim ticks we
     *                     are in decimal percent.
     */
    Renderer(SDL2pp::Renderer& inSdlRenderer, SDL2pp::Window& inWindow, UserInterface& inUI);

    /**
     * First renders all tiles in view, then renders all entities in view.
     */
    void render();

    /**
     * Handles window events.
     */
    bool handleEvent(SDL_Event& event) override;

private:
    SDL2pp::Renderer& sdlRenderer;
    UserInterface& ui;
};

} // namespace SpriteEditor
} // namespace AM
