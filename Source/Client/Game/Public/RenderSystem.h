#pragma once

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
    static constexpr unsigned int RENDER_TICKS_PER_SECOND = 60;
    static constexpr double RENDER_INTERVAL_S
        = 1.0 / static_cast<double>(RENDER_TICKS_PER_SECOND);

    /** An unreasonable amount of time for the render tick to be late by.
        Late render ticks cause jittering, as the pacing between ticks becomes
        inconsistent. */
    static constexpr double RENDER_DELAYED_TIME_S = .001;

    RenderSystem(SDL2pp::Renderer& inRenderer, Game& inGame,
                 SDL2pp::Window& inWindow);

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
