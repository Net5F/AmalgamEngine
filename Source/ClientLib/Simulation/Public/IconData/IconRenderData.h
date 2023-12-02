#pragma once

#include "AssetCache.h"
#include <SDL_rect.h>

namespace AM
{
namespace Client
{

/**
 * Holds a icon's rendering-related data.
 *
 * See Icon.h for more info.
 */
struct IconRenderData {
    /** The relative path to the icon sheet image file that holds this
        icon. Used for passing the icon to our UI library, which has its
        own texture cache. */
    std::string iconSheetRelPath{};

    /** UV position and size in texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};
};

} // End namespace Client
} // End namespace AM
