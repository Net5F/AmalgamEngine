#pragma once

#include "BoundingBox.h"
#include <SDL_rect.h>

struct SDL_Texture;

namespace AM
{
namespace ResourceImporter
{
class DataModel;
struct EditorSpriteSheet;

/**
 * Helper functions for working with sprite images.
 */
class SpriteTools 
{
public:
    /**
     * Calculates the world-space stage for the given sprite image.
     * The stage's origin will align with the given screen-space origin.
     *
     * Note: spriteImageExtent and stageOrigin should be the native values, 
     *       without any scaling.
     */
    static BoundingBox
        calcSpriteStageWorldExtent(const SDL_Rect& spriteImageExtent,
                                   const SDL_Point& stageOrigin);

    /**
     * Generates a sprite sheet texture containing all of the sheet's sprites, 
     * placed at their current textureExtents.
     */
    static SDL_Texture*
        generateSpriteSheetTexture(const DataModel& dataModel,
                                   const EditorSpriteSheet& spriteSheet);
};

} // End namespace ResourceImporter
} // End namespace AM
