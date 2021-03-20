#pragma once

#include "entt/core/hashed_string.hpp"
#include "entt/resource/cache.hpp"
#include "entt/resource/loader.hpp"
#include "SDL2pp/Texture.hh"
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
     */
    ResourceManager(SDL2pp::Renderer& sdlRenderer);

    /**
     * Attempts to load the texture at the given path.
     *
     * @param path  The path to the directory that contains the texture.
     * @param filename  The filename of the texture. Used as the identifier.
     * @return true if the texture was successfully loaded, else false.
     */
    bool loadTexture(std::string_view path, const entt::hashed_string& filename);

    /**
     * @param id  The resource identifier, a path including the full file name,
     *            relative to "Resources/Textures/".
     */
    entt::resource_handle<SDL2pp::Texture>
        getTexture(const entt::hashed_string id);

private:
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
