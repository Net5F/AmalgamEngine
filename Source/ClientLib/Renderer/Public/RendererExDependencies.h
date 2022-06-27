#pragma once

#include <functional>

struct SDL_Renderer;

namespace AM
{
class World;
class UserInterface;

namespace Client
{

/**
 * Defines the dependencies that will be injected into the project's
 * RendererExtension class.
 */
struct RendererExDependencies {
public:
    SDL_Renderer* sdlRenderer;

    World& world;

    UserInterface& userInterface;

    std::function<double(void)> getSimTickProgress;
};

} // namespace Client
} // namespace AM
