#include "Renderer.h"
#include "World.h"
#include "Sim.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Sprite.h"
#include "Camera.h"
#include "MovementHelpers.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Client
{
Renderer::Renderer(SDL2pp::Renderer& inSdlRenderer, Sim& inSim,
                           SDL2pp::Window& window)
: sdlRenderer(inSdlRenderer)
, sim(inSim)
, world(sim.getWorld())
, accumulatedTime(0.0)
{
    // Init the groups that we'll be using.
    auto group
        = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    ignore(group);

    // TODO: This will eventually be used when we get to variable window sizes.
    ignore(window);
}

void Renderer::tick()
{
    accumulatedTime += frameTimer.getDeltaSeconds(true);

    if (accumulatedTime >= RENDER_TICK_TIMESTEP_S) {
        // Process the rendering for this frame.
        render();

        accumulatedTime -= RENDER_TICK_TIMESTEP_S;
        if (accumulatedTime >= RENDER_TICK_TIMESTEP_S) {
            // If we've accumulated enough time to render again, something
            // happened (probably a window event that stopped app execution.)
            // We still only want to render the latest data, but it's worth
            // giving debug output that we detected this.
            LOG_INFO("Detected a request for two renders in the same frame. "
                     "Render must have been massively delayed. Render was "
                     "delayed by: %.8fs. Setting to 0.",
                     accumulatedTime);
            accumulatedTime = 0;
        }
    }
}

void Renderer::initTimer()
{
    frameTimer.updateSavedTime();
}

double Renderer::getTimeTillNextFrame()
{
    // The time since accumulatedTime was last updated.
    double timeSinceIteration = frameTimer.getDeltaSeconds(false);
    return (RENDER_TICK_TIMESTEP_S - (accumulatedTime + timeSinceIteration));
}

void Renderer::render()
{
    /* Set up the camera for this frame. */
    // Get how far we are between game ticks in decimal percent.
    double alpha = sim.getIterationProgress();

    // Get the lerped camera position based on the alpha.
    auto [playerCamera, playerSprite, playerPosition, playerPreviousPos]
        = world.registry.get<Camera, Sprite, Position, PreviousPosition>(
            world.playerEntity);
    Position cameraLerp = MovementHelpers::interpolatePosition(
        playerCamera.prevPosition, playerCamera.position, alpha);

    // Set the camera at the lerped position.
    Camera lerpedCamera = playerCamera;
    lerpedCamera.position.x = cameraLerp.x;
    lerpedCamera.position.y = cameraLerp.y;

    /* Render. */
    // Clear the screen to prepare for drawing.
    sdlRenderer.Clear();

    // Render the world tiles first.
    renderTiles(lerpedCamera);

    // Render all entities, lerping between their previous and next positions.
    renderEntities(lerpedCamera, alpha);

    sdlRenderer.Present();
}

void Renderer::renderTiles(Camera& camera)
{
    for (unsigned int y = 0; y < WORLD_HEIGHT; ++y) {
        for (unsigned int x = 0; x < WORLD_WIDTH; ++x) {
            // Get screen coords for this tile.
            ScreenPoint screenPos = tileToScreen(x, y);

            // Get the sprite's vertical offset (iso sprites may have extra
            // vertical space to show depth, we just want the tile.)
            Sprite& sprite{world.terrainMap[y * WORLD_WIDTH + x]};
            int spriteOffsetY = sprite.height - TILE_SCREEN_HEIGHT;

            // Get the coordinates to render at, adjusted based on the camera.
            int adjustedX = static_cast<int>((screenPos.x - camera.position.x));
            int adjustedY = static_cast<int>((screenPos.y - spriteOffsetY - camera.position.y));

            // Draw the tile.
            SDL2pp::Rect screenExtent = {adjustedX, adjustedY, sprite.width,
                    sprite.height};
            sdlRenderer.Copy(*(sprite.texturePtr), sprite.texExtent, screenExtent);
        }
    }
}

void Renderer::renderEntities(Camera& camera, double alpha)
{
    // Render all entities with a Sprite, PreviousPosition and Position.
    auto group
        = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos]
            = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the lerp'd world position and convert it to screen space.
        Position lerp = MovementHelpers::interpolatePosition(previousPos,
                                                             position, alpha);
        int screenX
            = static_cast<int>(lerp.x) - camera.position.x;
        int screenY
            = static_cast<int>(lerp.y) - camera.position.y;

        // If the sprite is within the camera bounds, render it.
        if ((screenX + sprite.width) > 0 && (screenY + sprite.height) > 0) {
            SDL2pp::Rect screenExtent
                = {screenX, screenY, sprite.width, sprite.height};

            sdlRenderer.Copy(*(sprite.texturePtr), sprite.texExtent,
                          screenExtent);
        }
    }
}

Renderer::ScreenPoint Renderer::tileToScreen(int x, int y) {
    // Convert tile index to isometric screen position.
    int screenX = (x - y) * (TILE_SCREEN_WIDTH / 2);
    int screenY = (x + y) * (TILE_SCREEN_HEIGHT / 2);

    return {screenX, screenY};
}

} // namespace Client
} // namespace AM
