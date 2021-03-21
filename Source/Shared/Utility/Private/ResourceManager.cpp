#include "ResourceManager.h"
#include "Log.h"

namespace AM
{
ResourceManager::ResourceManager(const std::string& inRunPath, SDL2pp::Renderer& inSdlRenderer)
: runPath(inRunPath)
, sdlRenderer(inSdlRenderer)
{
}

bool ResourceManager::loadTexture(std::string_view path, const entt::hashed_string& filename)
{
    // Prepare the path.
    std::string fullPath{runPath};
    fullPath += std::string(path);
    if (!(fullPath.ends_with("/"))) {
        fullPath += "/";
    }
    fullPath += filename.data();

    TextureHandle handle = textureCache.load<TextureLoader>(filename, fullPath, sdlRenderer);
    if (!handle) {
        LOG_ERROR("Failed to load texture at path: %s", (fullPath));
    }

    return true;
}

TextureHandle ResourceManager::getTexture(const entt::hashed_string id)
{
    TextureHandle handle = textureCache.handle(id);
    if (!handle) {
        LOG_ERROR("Requested invalid texture resource. id: %s", id.data());
    }

    return handle;
}

} // End namespace AM
