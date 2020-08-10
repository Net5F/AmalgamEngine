#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "SDL2pp/SDL2pp.hh"
#include "Timer.h"

namespace AM
{
namespace Client
{

class Game;
class World;

class RenderSystem
{
public:
    /** 60 rendered frames per second. */
    static const Uint64 RENDER_INTERVAL_COUNT;

    /** An unreasonable amount of time for the render tick to be late by.
        Late render ticks cause jittering, as the pacing between ticks becomes
        inconsistent. */
    static const Uint64 RENDER_DELAYED_TIME_COUNT;

    RenderSystem(SDL2pp::Renderer& inRenderer, Game& inGame, SDL2pp::Window& inWindow);

    void render(Uint64 deltaCount);

    Uint64 getAccumulatedCount();

private:
    SDL2pp::Renderer& renderer;
    Game& game;
    World& world;

    /** The aggregated time since we last processed a tick. */
    Uint64 accumulatedCount;
};

} // namespace Client
} // namespace AM

#endif /* RENDERSYSTEM_H */
