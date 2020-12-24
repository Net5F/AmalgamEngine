#include "RenderSystem.h"
#include "World.h"
#include "Game.h"
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
RenderSystem::RenderSystem(SDL2pp::Renderer& inRenderer, Game& inGame,
                           SDL2pp::Window& window)
: renderer(inRenderer)
, game(inGame)
, world(game.getWorld())
, accumulatedTime(0.0)
{
    // Init the groups that we'll be using.
    auto group = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    ignore(group);

    // TODO: This will eventually be used when we get to variable window sizes.
    ignore(window);
}

void RenderSystem::tick()
{
    accumulatedTime += frameTimer.getDeltaSeconds(true);

    if (accumulatedTime >= RENDER_TICK_TIMESTEP_S) {
        // How far we are between game ticks in decimal percent.
        double alpha = game.getIterationProgress();

        // Process the rendering for this frame.
        render(alpha);

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

void RenderSystem::initTimer()
{
    frameTimer.updateSavedTime();
}

double RenderSystem::getTimeTillNextFrame()
{
    // The time since accumulatedTime was last updated.
    double timeSinceIteration = frameTimer.getDeltaSeconds(false);
    return (RENDER_TICK_TIMESTEP_S - (accumulatedTime + timeSinceIteration));
}

void RenderSystem::render(double alpha)
{
    // Clear the screen to prepare for drawing.
    renderer.Clear();

    // Get the offset between world and screen space.
    auto [playerCamera, playerSprite, playerPosition, playerPreviousPos] = world.registry.get<Camera, Sprite, Position, PreviousPosition>(world.playerEntity);
    Position playerLerp = MovementHelpers::interpolatePosition(playerPreviousPos, playerPosition, alpha);

    int worldToScreenOffsetX = playerLerp.x - (playerCamera.width / 2)
                               + (playerSprite.width / 2);
    int worldToScreenOffsetY = playerLerp.y - (playerCamera.height / 2)
                               + (playerSprite.height / 2);

    // Render all entities with a Sprite, PreviousPosition and Position.
    auto group = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos] = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the lerp'd world position and convert it to screen space.
        Position lerp = MovementHelpers::interpolatePosition(previousPos, position, alpha);
        int screenX = static_cast<int>(std::floor(lerp.x)) - worldToScreenOffsetX;
        int screenY = static_cast<int>(std::floor(lerp.y)) - worldToScreenOffsetY;

        // If the sprite is within the camera bounds, render it.
        if ((screenX + sprite.width) > 0 && (screenY + sprite.height) > 0) {
            SDL2pp::Rect spriteRect
                = {screenX, screenY, sprite.width, sprite.height};

            renderer.Copy(*(sprite.texturePtr), sprite.posInTexture, spriteRect);
        }
    }

    renderer.Present();
}

} // namespace Client
} // namespace AM
