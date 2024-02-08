#include "Renderer.h"
#include "UserInterface.h"
#include "Log.h"
#include <SDL_render.h>
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace ResourceImporter
{
Renderer::Renderer(SDL_Renderer* inSdlRenderer, UserInterface& inUI)
: sdlRenderer{inSdlRenderer}
, ui{inUI}
{
}

void Renderer::render()
{
    /* Render. */
    // Clear the current rendering target to prepare for rendering.
    SDL_RenderClear(sdlRenderer);

    // Render the current UI screen.
    ui.render();

    // Present the finished back buffer to the user's screen.
    SDL_RenderPresent(sdlRenderer);
}

bool Renderer::handleOSEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_WINDOWEVENT:
            // TODO: Handle this.
            return true;
    }

    return false;
}

} // namespace ResourceImporter
} // namespace AM
