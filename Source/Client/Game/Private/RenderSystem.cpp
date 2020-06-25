#include "RenderSystem.h"
#include "World.h"
#include "Game.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

RenderSystem::RenderSystem(SDL2pp::Renderer& inRenderer, Game& inGame,
                           SDL2pp::Window& window)
: renderer(inRenderer), game(inGame), world(game.getWorld()), accumulatedTime(0.0f)
{
}

void RenderSystem::render(float deltaSeconds)
{
    accumulatedTime += deltaSeconds;

    // Process as many game ticks as have accumulated.
    if (accumulatedTime >= RENDER_INTERVAL_S) {
        renderer.Clear();

        // How far we are between game ticks in decimal percent.
        float alpha = game.getAccumulatedTime() / GAME_TICK_INTERVAL_S;
        for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
            if (world.entityExists(entityID)) {
                const SpriteComponent& sprite = world.sprites[entityID];
                const PositionComponent& position = world.positions[entityID];
                const PositionComponent& oldPosition = world.oldPositions[entityID];

                // Lerp'd position based on how far we are between game ticks.
                // TODO: Have a real conversion instead of casting to int here.
                int lerpX = (position.x * alpha) + (oldPosition.x * (1.0 - alpha));
                int lerpY = (position.y * alpha) + (oldPosition.y * (1.0 - alpha));
                SDL2pp::Rect spriteWorldData = { lerpX, lerpY, sprite.width,
                        sprite.height };

                renderer.Copy(*(world.sprites[entityID].texturePtr),
                    world.sprites[entityID].posInTexture, spriteWorldData);
            }
        }

        renderer.Present();

        accumulatedTime -= RENDER_INTERVAL_S;
        if (accumulatedTime >= RENDER_INTERVAL_S) {
            // If we've accumulated enough time to render more, something
            // happened (probably a window event that stopped app execution.)
            // We still only want to render the latest data, but it's worth giving
            // debug output that we detected this.
            DebugInfo(
                "Detected a delayed render. accumulatedTime: %f. Setting to 0.",
                accumulatedTime);
            accumulatedTime = 0;
        }
    }

}

float RenderSystem::getAccumulatedTime()
{
    return accumulatedTime;
}

} // namespace Client
} // namespace AM
