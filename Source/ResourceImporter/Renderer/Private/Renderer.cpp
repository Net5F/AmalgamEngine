#include "Renderer.h"
#include "UserInterface.h"
#include "Log.h"
#include <SDL3/SDL_render.h>

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
    //switch (event.type) {
		// TODO: Handle window events.
    //}

    return false;
}

} // namespace ResourceImporter
} // namespace AM
