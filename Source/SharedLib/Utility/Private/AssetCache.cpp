#include "AssetCache.h"
#include "Log.h"

namespace AM
{
AssetCache::AssetCache(SDL_Renderer* inSdlRenderer)
: sdlRenderer{inSdlRenderer}
{
}

TextureHandle AssetCache::loadTexture(const std::string& imagePath)
{
    // If the texture is already loaded, return it.
    auto it{textureCache.find(imagePath)};
    if (it != textureCache.end()) {
        return it->second;
    }

    // Load the texture.
    SDL_Texture* texture{IMG_LoadTexture(sdlRenderer, imagePath.c_str())};
    if (texture == nullptr) {
        LOG_FATAL("Failed to load texture: %s", imagePath.c_str());
    }

    // Wrap the texture in a shared_ptr.
    TextureHandle handle{
        TextureHandle(texture, [](SDL_Texture* p) { SDL_DestroyTexture(p); })};

    // Save the texture in the cache.
    textureCache[imagePath] = handle;

    return handle;
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
