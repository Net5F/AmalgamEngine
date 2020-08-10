#include "RenderSystem.h"
#include "World.h"
#include "Game.h"
#include "Debug.h"
#include "Ignore.h"

namespace AM
{
namespace Client
{

const Uint64 RenderSystem::RENDER_INTERVAL_COUNT = Timer::ipsToCount(60);
const Uint64 RenderSystem::RENDER_DELAYED_TIME_COUNT = Timer::secondsToCount(.001);

RenderSystem::RenderSystem(SDL2pp::Renderer& inRenderer, Game& inGame,
                           SDL2pp::Window& window)
: renderer(inRenderer), game(inGame), world(game.getWorld()), accumulatedCount(0)
{
    // TODO: This will eventually be used when we get to variable window sizes.
    ignore(window);
}

void RenderSystem::render(Uint64 deltaCount)
{
    accumulatedCount += deltaCount;

    // Process the rendering for this frame.
    if (accumulatedCount >= RENDER_INTERVAL_COUNT) {
        renderer.Clear();

        // How far we are between game ticks in decimal percent.
        double alpha = static_cast<double>(game.getAccumulatedCount())
        / GAME_TICK_INTERVAL_COUNT;
        for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
            if (world.entityExists(entityID)) {
                const SpriteComponent& sprite = world.sprites[entityID];
                const PositionComponent& position = world.positions[entityID];
                const PositionComponent& oldPosition = world.oldPositions[entityID];

                // Lerp'd position based on how far we are between game ticks.
                int lerpX = std::round((position.x * alpha) + (oldPosition.x * (1.0 - alpha)));
                int lerpY = std::round((position.y * alpha) + (oldPosition.y * (1.0 - alpha)));
                SDL2pp::Rect spriteWorldData = { lerpX, lerpY, sprite.width,
                        sprite.height };

                renderer.Copy(*(world.sprites[entityID].texturePtr),
                    world.sprites[entityID].posInTexture, spriteWorldData);
            }
        }

        renderer.Present();

        accumulatedCount -= RENDER_INTERVAL_COUNT;
        if (accumulatedCount >= RENDER_INTERVAL_COUNT) {
            // If we've accumulated enough time to render again, something
            // happened (probably a window event that stopped app execution.)
            // We still only want to render the latest data, but it's worth giving
            // debug output that we detected this.
            DebugInfo(
                "Detected a request for two renders in the same frame. Render must have"
                "been massively delayed. Render was delayed by: %.8fs. Setting to 0.",
                accumulatedCount);
            accumulatedCount = 0;
        }
        else if (accumulatedCount >= RENDER_DELAYED_TIME_COUNT) {
            // Delayed render could be caused by the sim taking too long, or too much printing.
            DebugInfo("Detected a delayed render. Render was delayed by: %.8fs.",
                accumulatedCount);
        }
    }

}

Uint64 RenderSystem::getAccumulatedCount()
{
    return accumulatedCount;
}

} // namespace Client
} // namespace AM
