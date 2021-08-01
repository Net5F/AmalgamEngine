#pragma once

#include "SpriteRenderInfo.h"
#include "entt/entity/registry.hpp"
#include <vector>

class SDL_Rect;

namespace AM
{
class Camera;
namespace Client
{

/**
 * This class is responsible for figuring out which sprites from the world are
 * relevant to the current frame and providing them in a sorted, prepared form.
 */
class WorldSpritePreparer
{
public:
    // TODO: Turn the map into its own data type.
    using TileLayer = std::vector<Sprite>;
    WorldSpritePreparer(entt::registry& inRegistry,
                        std::vector<TileLayer>& inMapLayers);

    /**
     * Clears the stored sprite info and prepares the updated batch of sprites.
     *
     * @return A reference to a vector of all world sprites that should be
     *         drawn on this frame, sorted into ascending draw order.
     */
    std::vector<SpriteRenderInfo>& prepareSprites(const Camera& camera,
                                                  double alpha);

private:
    /**
     * Updates the world bounds for any dynamic entity that has a Sprite.
     * (i.e. any entity with a Sprite, Position, and PreviousPosition)
     */
    void updateSpriteWorldBounds(double alpha);

    /**
     * Gathers the sprites that should be drawn on this frame, calculates their
     * screen position, and pushes their data into the given vector.
     *
     * @param camera  The camera to calculate screen position with.
     * @param alpha  For entities, the alpha to lerp between positions with.
     *
     * @post sprites is filled with the sprites that are relevant to this
     *       frame.
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
    void visitSprite(SpriteRenderInfo& spriteInfo, int& depthValue);

    /**
     * Returns true if the given extent is within the given camera's bounds,
     * else false.
     *
     * @param extent  An extent in final screen coordinates.
     * @param camera  The camera to use for screen width/height checks.
     */
    bool isWithinScreenBounds(const SDL_Rect& extent, const Camera& camera);

    /** Registry reference used for gathering sprites. */
    entt::registry& registry;
    /** Tile map reference used for gathering tiles. */
    std::vector<TileLayer>& mapLayers;

    /** Stores the sorted sprite info from the last prepareSprites() call.
        Calculations and sorting are done in-place.
        Indices 0 - heightfulSpriteStartIndex are heightless (flat tiles),
        the rest are sprites with height. */
    std::vector<SpriteRenderInfo> sprites;

    /** The start index for sprites in the sprites vector that have height. */
    unsigned int heightfulSpriteStartIndex;
};

} // End namespace Client
} // End namespace AM
