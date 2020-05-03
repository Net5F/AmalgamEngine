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
    static constexpr float RENDER_INTERVAL_S = 1 / 60.0f;

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
