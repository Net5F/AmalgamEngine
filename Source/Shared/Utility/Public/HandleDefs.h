#pragma once

#include "entt/resource/handle.hpp"
#include "SDL2pp/Texture.hh"

/**
 * This file contains convenience aliases for resource handles (since
 * entt::resource_handle<TypeName> is very verbose).
 *
 * When you only need to interface with the handle and not the broader
 * ResourceManager, include this header.
 */
namespace AM
{

using TextureHandle = entt::resource_handle<SDL2pp::Texture>;

} // End namespace AM
