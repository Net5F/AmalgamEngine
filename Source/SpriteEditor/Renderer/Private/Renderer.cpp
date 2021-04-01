#include "Renderer.h"
#include "UserInterface.h"
#include "Log.h"
#include "Ignore.h"

#include <SDL2/SDL2_gfxPrimitives.h>

namespace AM
{
namespace SpriteEditor
{
Renderer::Renderer(SDL2pp::Renderer& inSdlRenderer, SDL2pp::Window& window, UserInterface& inUI)
: sdlRenderer(inSdlRenderer)
, ui(inUI)
{
    // TODO: This will eventually be used when we get to variable window sizes.
    ignore(window);
}

unsigned int i = 0;
void Renderer::render()
{
    /* Render. */
    // Clear the screen to prepare for drawing.
    sdlRenderer.Clear();

    sdlRenderer.SetDrawColor(i, i, i);
    i = (i + 1) % 255;

    // Render the finished buffer to the screen.
    sdlRenderer.Present();
}

bool Renderer::handleEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_WINDOWEVENT:
            // TODO: Handle this.
            return true;
    }

    return false;
}

} // namespace SpriteEditor
} // namespace AM
