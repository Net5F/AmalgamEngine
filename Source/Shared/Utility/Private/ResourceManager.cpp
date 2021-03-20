#include "ResourceManager.h"
#include "Log.h"

namespace AM
{
ResourceManager::ResourceManager(SDL2pp::Renderer& inSdlRenderer)
: sdlRenderer(inSdlRenderer)
{
}

bool ResourceManager::loadTexture(std::string_view path, const entt::hashed_string& filename)
{
    // Prepare the path.
    std::string fullPath{path};
    if (!(fullPath.ends_with("/"))) {
        fullPath += "/";
    }
    fullPath += filename.data();

    entt::resource_handle<SDL2pp::Texture> handle = textureCache.load<TextureLoader>(filename, fullPath, sdlRenderer);
    if (!handle) {
        LOG_ERROR("Failed to load texture at path: %s", (fullPath));
    }

    return true;
}

entt::resource_handle<SDL2pp::Texture>
    ResourceManager::getTexture(const entt::hashed_string id)
{
    entt::resource_handle<SDL2pp::Texture> handle = textureCache.handle(id);
    if (!handle) {
        LOG_ERROR("Requested invalid texture resource. id: %s", id.data());
    }

    return handle;
}

} // End namespace AM
