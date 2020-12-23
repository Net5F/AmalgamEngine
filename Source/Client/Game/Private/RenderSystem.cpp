#include "RenderSystem.h"
#include "World.h"
#include "Game.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Sprite.h"
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
        /* Process the rendering for this frame. */
        renderer.Clear();

        // How far we are between game ticks in decimal percent.
        const double alpha = game.getIterationProgress();

        // Render all entities with a Sprite, PreviousPosition and Position.
        auto group = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
        for (entt::entity entity : group) {
            const Sprite& sprite = group.get<Sprite>(entity);
            const Position& position = group.get<Position>(entity);
            const PreviousPosition& previousPos = group.get<PreviousPosition>(entity);

            // Lerp'd position based on how far we are between game ticks.
            const double interpX
                = (position.x * alpha) + (previousPos.x * (1.0 - alpha));
            const double interpY
                = (position.y * alpha) + (previousPos.y * (1.0 - alpha));
            const int lerpX = static_cast<int>(std::floor(interpX));
            const int lerpY = static_cast<int>(std::floor(interpY));
            SDL2pp::Rect spriteWorldData
                = {lerpX, lerpY, sprite.width, sprite.height};

            renderer.Copy(*(sprite.texturePtr), sprite.posInTexture, spriteWorldData);
        }

        renderer.Present();

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

} // namespace Client
} // namespace AM
