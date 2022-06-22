#include "RendererHooks.h"
#include "World.h"
#include "Camera.h"

#include <SDL_render.h>

namespace AM
{
namespace Client
{

void RendererHooks::setSdlRenderer(SDL_Renderer* inSdlRenderer)
{
    sdlRenderer = inSdlRenderer;
}

void RendererHooks::setWorld(const World* inWorld)
{
    world = inWorld;
}

} // namespace Client
} // namespace AM
