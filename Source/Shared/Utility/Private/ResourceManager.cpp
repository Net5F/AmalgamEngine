#include "ResourceManager.h"
#include "Log.h"

namespace AM
{

ResourceManager::ResourceManager(SDL2pp::Renderer& sdlRenderer)
{
    std::string path{"Resources/Textures/"};
    entt::hashed_string id{"iso_test_sprites.png"};
    textureCache.load<TextureLoader>(id, (path + id.data()), sdlRenderer);
}

entt::resource_handle<SDL2pp::Texture> ResourceManager::getTexture(const entt::hashed_string id)
{
    entt::resource_handle<SDL2pp::Texture> handle = textureCache.handle(id);
    if (!handle) {
        LOG_ERROR("Requested invalid texture resource. id: %s", id.data());
    }

    return handle;
}

} // End namespace AM
