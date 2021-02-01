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

    // Get iso screen coords for the center point of camera.
    SDL2pp::Point lerpedCameraCenter = worldToScreen(lerpedCamera.position.x, lerpedCamera.position.y);

    // Calc where the top left of the lerpedCamera is in screen space.
    lerpedCamera.extent.x = lerpedCameraCenter.x - (lerpedCamera.extent.w / 2);
    lerpedCamera.extent.y = lerpedCameraCenter.y - (lerpedCamera.extent.h / 2);

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
            // Get iso screen coords for this tile.
            SDL2pp::Point tileScreenPos = tileToScreen(x, y);

            // Get the sprite's vertical offset (iso sprites may have extra
            // vertical space to show depth, we just want the tile.)
            Sprite& sprite{world.terrainMap[y * WORLD_WIDTH + x]};
            int spriteOffsetY = sprite.height - TILE_SCREEN_HEIGHT
                                - TILE_SCREEN_EDGE_HEIGHT;

            // Apply the camera adjustment.
            int adjustedX = static_cast<int>((tileScreenPos.x - camera.extent.x) * camera.zoomFactor);
            int adjustedY = static_cast<int>((tileScreenPos.y - spriteOffsetY - camera.extent.y) * camera.zoomFactor);

            // Prep the sprite data, accounting for camera zoom.
            int zoomedWidth = static_cast<int>(sprite.width * camera.zoomFactor);
            int zoomedHeight = static_cast<int>(sprite.height * camera.zoomFactor);
            SDL2pp::Rect screenExtent =
                {adjustedX, adjustedY, zoomedWidth, zoomedHeight};

            // Draw the tile.
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

        // Get the lerp'd world position and translate it to iso screen space.
        Position lerp = MovementHelpers::interpolatePosition(previousPos,
                                                             position, alpha);
        SDL2pp::Point entityScreenPos = worldToScreen(lerp.x, lerp.y);

        // Apply the camera adjustment to the entity.
        int adjustedX
            = static_cast<int>((entityScreenPos.x - camera.extent.x) * camera.zoomFactor);
        int adjustedY
            = static_cast<int>((entityScreenPos.y - camera.extent.y) * camera.zoomFactor);

        // If the entity's sprite is within the camera bounds, render it.
        if (isWithinCameraBounds(adjustedX, adjustedY, sprite, camera.extent)) {
            // Prep the sprite data, accounting for camera zoom.
            int zoomedWidth = static_cast<int>(sprite.width * camera.zoomFactor);
            int zoomedHeight = static_cast<int>(sprite.height * camera.zoomFactor);
            SDL2pp::Rect screenExtent
                = {adjustedX, adjustedY, zoomedWidth, zoomedHeight};

            sdlRenderer.Copy(*(sprite.texturePtr), sprite.texExtent,
                          screenExtent);
        }
    }
}

SDL2pp::Point Renderer::tileToScreen(int xIndex, int yIndex) {
    // Convert tile index to isometric screen position.
    int screenX = (xIndex - yIndex) * (TILE_SCREEN_WIDTH / 2);
    int screenY = (xIndex + yIndex) * (TILE_SCREEN_HEIGHT / 2);

    return {screenX, screenY};
}

SDL2pp::Point Renderer::worldToScreen(float x, float y) {
    // Calc the scale diff between world tile and screen tile sizes.
    static const float TILE_WIDTH_SCALE = TILE_SCREEN_WIDTH / TILE_WORLD_WIDTH;
    static const float TILE_HEIGHT_SCALE = TILE_SCREEN_HEIGHT / TILE_WORLD_HEIGHT;

    // Convert cartesian world point to isometric screen point.
    int screenX = static_cast<int>((x - y) * TILE_WIDTH_SCALE / 2) + (TILE_SCREEN_WIDTH / 2);
    int screenY = static_cast<int>((x + y)  * TILE_HEIGHT_SCALE / 2);

    return {screenX, screenY};
}

bool Renderer::isWithinCameraBounds(int x, int y, Sprite& sprite, SDL2pp::Rect& cameraBounds)
{
    // If the sprite is within view of the camera, return true.
    if ((x + sprite.width > cameraBounds.x) && (x < static_cast<int>(cameraBounds.w))
        && (y + sprite.height > cameraBounds.y) && (y < static_cast<int>(cameraBounds.h))) {
        return true;
    }
    else {
        return false;
    }
}

} // namespace Client
} // namespace AM
