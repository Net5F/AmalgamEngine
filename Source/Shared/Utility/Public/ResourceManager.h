#pragma once

#include "TextureHandle.h"
#include "entt/core/hashed_string.hpp"
#include "entt/resource/cache.hpp"
#include "entt/resource/loader.hpp"
#include "SDL2pp/Renderer.hh"
#include <string_view>
#include <memory>

namespace AM
{

/**
 * This class facilitates loading and managing the lifetime of texture and
 * audio resources.
 */
class ResourceManager
{
public:
    /**
     * Parses the resource file, loading all listed resources.
     *
     * @param inRunPath  The path that the executable was started from. Used when
     *                   building the resource file path.
     * @param sdlRenderer  The renderer to construct Texture resources with.
     */
    ResourceManager(const std::string& inRunPath, SDL2pp::Renderer& sdlRenderer);

    /**
     * Attempts to load the texture at the given path.
     *
     * @param path  The path to the directory that contains the texture,
     *              starting from the executable location.
     * @param filename  The filename of the texture. Used as the identifier.
     * @return true if the texture was successfully loaded, else false.
     */
    bool loadTexture(std::string_view path, const entt::hashed_string& filename);

    /**
     * @param id  The resource identifier, a path including the full file name
     * @return A valid handle if the id exists, else an invalid handle.
     */
     TextureHandle getTexture(const entt::hashed_string id);

private:
    /* The path that the executable was started from. Used when constructing
       the resource file path. */
    const std::string runPath;

    /** SDL2 renderer reference, used for constructing textures. */
    SDL2pp::Renderer& sdlRenderer;

    entt::resource_cache<SDL2pp::Texture> textureCache;
};

/**
 * Specialized loader for SDL2pp::Texture.
 */
struct TextureLoader : entt::resource_loader<TextureLoader, SDL2pp::Texture> {
    std::shared_ptr<SDL2pp::Texture> load(const std::string& filename,
                                          SDL2pp::Renderer& sdlRenderer) const
    {
        return std::make_shared<SDL2pp::Texture>(
            SDL2pp::Texture(sdlRenderer, filename));
    }
};

} // End namespace AM
