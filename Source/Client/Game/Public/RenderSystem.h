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
    static constexpr double RENDER_INTERVAL_S = 1 / 60.0;

    /** An unreasonable amount of time for the render tick to be late by.
        Late render ticks cause jittering, as the pacing between ticks becomes
        inconsistent. */
    static constexpr double RENDER_DELAYED_TIME_S = .001;

    RenderSystem(SDL2pp::Renderer& inRenderer, Game& inGame, SDL2pp::Window& inWindow);

    void render();

    /** Initialize the frame timer. */
    void initTimer();

    double getAccumulatedTime();

private:
    SDL2pp::Renderer& renderer;
    Game& game;
    World& world;

    /** Used to time when we should render a frame. */
    Timer frameTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;
};

} // namespace Client
} // namespace AM

#endif /* RENDERSYSTEM_H */
