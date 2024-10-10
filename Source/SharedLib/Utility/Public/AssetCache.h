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
     * If a texture with the given resource ID is in the cache, returns it.
     * If not and the resource ID is a valid file path to an image, adds the 
     * image to the cache and returns it.
     *
     * @param resourceID An abstract resource ID (for textures added using 
     *                   addTexture()), or the full path to an image file.
     * @return A valid texture if one was found, else nullptr.
     */
    std::shared_ptr<SDL_Texture> requestTexture(const std::string& resourceID);

    /**
     * Adds the given texture to the cache, using the given ID.
     * If a texture already exists with the given ID, it will be overwritten.
     *
     * Note: The texture will be copied, so the given pointer does not need 
     *       to remain valid.
     * @return A managed copy of the given texture.
     */
    std::shared_ptr<SDL_Texture> addTexture(SDL_Texture* rawTexture,
                                            const std::string& resourceID);

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
