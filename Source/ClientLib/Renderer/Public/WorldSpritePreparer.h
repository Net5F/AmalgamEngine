#pragma once

#include "SpriteSortInfo.h"
#include "entt/entity/registry.hpp"
#include <vector>

struct SDL_Rect;

namespace AM
{
struct Camera;

namespace Client
{
class TileMap;
class SpriteData;

/**
 * This class is responsible for figuring out which sprites from the world are
 * relevant to the current frame and providing them in a sorted, prepared form.
 */
class WorldSpritePreparer
{
public:
    WorldSpritePreparer(entt::registry& inRegistry, const TileMap& inTileMap,
                        const SpriteData& inSpriteData);

    /**
     * Clears the stored sprite info and prepares the updated batch of sprites.
     *
     * @return A reference to a vector of all sprites that should be drawn
     *         on this frame, sorted in their proper draw order.
     */
    std::vector<SpriteSortInfo>& prepareSprites(const Camera& camera,
                                                double alpha);

private:
    /**
     * Gathers the sprites that should be drawn on this frame, calculates their
     * screen position and bounding box for this frame, and pushes their data
     * into the sprites vector.
     *
     * @param camera  The camera to calculate screen position with.
     * @param alpha  For entities, the alpha to lerp between positions with.
     *
     * @post sprites is filled with the sprites that are relevant to this
     *       frame.
     * @post sortedSprites is filled with the ground sprites, and spritesToSort
     *       is filled with the sprites that need to be sorted.
     */
    void gatherSpriteInfo(const Camera& camera, double alpha);

    /**
     * Sorts the sprites into their draw order (farthest sprite first).
     */
    void sortSpritesByDepth();

    /**
     * Determines which sprites are behind eachother, from the viewer's
     * perspective. Necessary for depth sorting.
     *
     * @post Each element.spritesBehind in sprites is filled with the sprites
     *       that are behind that element's sprite.
     */
    void calcDepthDependencies();

    /**
     * Recursive function for finding sprite depth. Used by sortSpritesByDepth.
     *
     * @param spriteInfo  The sprite that we're visiting.
     * @param depthValue  The next depth value to assign.
     */
    void visitSprite(SpriteSortInfo& spriteInfo, int& depthValue);

    /**
     * Returns true if any part of the given extent is within the given
     * camera's bounds, else false.
     *
     * @param extent  An extent in final screen coordinates.
     * @param camera  The camera to use for screen width/height checks.
     */
    bool isWithinScreenBounds(const SDL_Rect& extent, const Camera& camera);

    /** Used for gathering sprites. */
    entt::registry& registry;

    /** Used for gathering tiles. */
    const TileMap& tileMap;

    /** Used for getting sprite render data. */
    const SpriteData& spriteData;

    /** Stores the sorted sprite info from the last prepareSprites() call.
        Calculations and sorting are done in-place.
        Indices 0 - boxSpriteStartIndex have no bounding boxes (floors,
        carpets, etc). The rest are sprites with bounding boxes. */
    std::vector<SpriteSortInfo> sortedSprites;

    /** Holds sprites that need to be sorted. Sprites are pushed during
        gatherSpriteInfo() and sorted during sortSpritesByDepth(). */
    std::vector<SpriteSortInfo> spritesToSort;
};

} // End namespace Client
} // End namespace AM
