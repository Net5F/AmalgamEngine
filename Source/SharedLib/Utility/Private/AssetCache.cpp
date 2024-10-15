#include "AssetCache.h"
#include "Log.h"

namespace AM
{
AssetCache::AssetCache(SDL_Renderer* inSdlRenderer)
: sdlRenderer{inSdlRenderer}
{
}

std::shared_ptr<SDL_Texture>
    AssetCache::requestTexture(const std::string& resourceID)
{
    // If the texture is already in the cache, return it.
    auto it{textureCache.find(resourceID)};
    if (it != textureCache.end()) {
        return it->second;
    }

    // The ID wasn't found in the cache, assume it's a path to an image and 
    // try to load it.
    SDL_Texture* rawTexture{IMG_LoadTexture(sdlRenderer, resourceID.c_str())};
    if (!rawTexture) {
        LOG_ERROR("Failed to load texture: %s", resourceID.c_str());
        return nullptr;
    }

    // Wrap the texture in a shared_ptr.
    std::shared_ptr<SDL_Texture> texture{
        rawTexture, [](SDL_Texture* p) { SDL_DestroyTexture(p); }};

    // Save the texture in the cache.
    textureCache[resourceID] = texture;

    return texture;
}

std::shared_ptr<SDL_Texture>
    AssetCache::addTexture(SDL_Texture* rawTexture,
                           const std::string& resourceID)
{
    // Wrap the texture in a shared_ptr.
    std::shared_ptr<SDL_Texture> texture{
        rawTexture, [](SDL_Texture* p) { SDL_DestroyTexture(p); }};

    // Save the texture in the cache.
    textureCache[resourceID] = texture;

    return texture;
}

bool AssetCache::discardTexture(const std::string& imagePath)
{
    // If the cache contains the given texture, discard it.
    auto it{textureCache.find(imagePath)};
    if (it != textureCache.end()) {
        textureCache.erase(it);
        return true;
    }
    else {
        return false;
    }
}

} // End namespace AM
