#pragma once

#include <functional>

struct SDL_Renderer;

namespace AM
{
class World;
class UserInterface;
class GraphicData;

namespace Client
{

/**
 * Provides the dependencies that Renderer objects may use.
 */
struct RendererContext {
public:
    SDL_Renderer* sdlRenderer;

    World& world;

    UserInterface& userInterface;

    /** Returns how far the sim tick is towards its next call, as a 
        percentage from 0 - 1. */
    std::function<double(void)> getSimTickProgress;

    GraphicData& graphicData;
};

} // namespace Client
} // namespace AM
