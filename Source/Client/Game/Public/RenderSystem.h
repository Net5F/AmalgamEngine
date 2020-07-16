#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "SDL2pp/SDL2pp.hh"

namespace AM
{
namespace Client
{

class Game;
class World;

class RenderSystem
{
public:
    /** Our rendering framerate. */
    static constexpr float RENDER_INTERVAL_S = 1 / 60.0f;

    /** An unreasonable amount of time for the render tick to be late by.
        Late render ticks cause jittering, as the pacing between ticks becomes
        inconsistent. */
    static constexpr float RENDER_DELAYED_TIME_S = .001;

    RenderSystem(SDL2pp::Renderer& inRenderer, Game& inGame, SDL2pp::Window& inWindow);

    void render(float deltaSeconds);

    float getAccumulatedTime();

private:
    SDL2pp::Renderer& renderer;
    Game& game;
    World& world;

    /** The aggregated time since we last processed a tick. */
    float accumulatedTime;
};

} // namespace Client
} // namespace AM

#endif /* RENDERSYSTEM_H */
