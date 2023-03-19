#pragma once

#include "Log.h"

#include <SDL_render.h>
#include <SDL_image.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace AM
{
// I don't like obfuscating the shared_ptr, but this alias is useful in case
// we decide to change the type.
using TextureHandle = std::shared_ptr<SDL_Texture>;

/**
 * Facilitates loading and managing the lifetime of assets.
 */
class AssetCache
{
public:
    /**
     * @param inSdlRenderer  The renderer to load textures with.
     */
    AssetCache(SDL_Renderer* inSdlRenderer);

    /**
     * Loads the image file at the given path into a texture and returns a
     * handle to it.
     *
     * If the texture is already loaded, returns a handle to it without re-
     * loading.
     *
     * @param imagePath  The image file's full path.
     */
    TextureHandle loadTexture(const std::string& imagePath);

    /**
     * Removes the texture associated with the given path from the cache.
     *
     * @param imagePath  The original image file's full path.
     * @return true if the texture was found and removed, else false.
     */
    bool discardTexture(const std::string& imagePath);

private:
    SDL_Renderer* sdlRenderer;

    std::unordered_map<std::string, TextureHandle> textureCache;
};

} // End namespace AM
