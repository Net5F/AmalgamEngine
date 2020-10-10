#include "RenderSystem.h"
#include "World.h"
#include "Game.h"
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
    // TODO: This will eventually be used when we get to variable window sizes.
    ignore(window);
}

void RenderSystem::render()
{
    accumulatedTime += frameTimer.getDeltaSeconds(true);

    // Process the rendering for this frame.
    if (accumulatedTime >= RENDER_INTERVAL_S) {
        renderer.Clear();

        // How far we are between game ticks in decimal percent.
        const double alpha = game.getIterationProgress();
        for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
            if (world.entityExists(entityID)) {
                const SpriteComponent& sprite = world.sprites[entityID];
                const PositionComponent& position = world.positions[entityID];
                const PositionComponent& oldPosition
                    = world.oldPositions[entityID];

                // Lerp'd position based on how far we are between game ticks.
                const double interpX
                    = (position.x * alpha) + (oldPosition.x * (1.0 - alpha));
                const double interpY
                    = (position.y * alpha) + (oldPosition.y * (1.0 - alpha));
                const int lerpX = static_cast<int>(std::floor(interpX));
                const int lerpY = static_cast<int>(std::floor(interpY));
                SDL2pp::Rect spriteWorldData
                    = {lerpX, lerpY, sprite.width, sprite.height};

                renderer.Copy(*(world.sprites[entityID].texturePtr),
                              world.sprites[entityID].posInTexture,
                              spriteWorldData);
            }
        }

        renderer.Present();

        accumulatedTime -= RENDER_INTERVAL_S;
        if (accumulatedTime >= RENDER_INTERVAL_S) {
            // If we've accumulated enough time to render again, something
            // happened (probably a window event that stopped app execution.)
            // We still only want to render the latest data, but it's worth
            // giving debug output that we detected this.
            LOG_INFO("Detected a request for two renders in the same frame. "
                     "Render must have been massively delayed. Render was "
                     "delayed by: {:.8f}s. Setting to 0.",
                     accumulatedTime);
            accumulatedTime = 0;
        }
    }
}

void RenderSystem::initTimer()
{
    frameTimer.updateSavedTime();
}

double RenderSystem::getAccumulatedTime()
{
    return accumulatedTime;
}

} // namespace Client
} // namespace AM
