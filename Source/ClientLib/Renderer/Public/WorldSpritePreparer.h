#pragma once

#include "SpriteSortInfo.h"
#include "TileLayers.h"
#include "PhantomTileSpriteInfo.h"
#include "TileSpriteColorModInfo.h"
#include "entt/entity/registry.hpp"
#include <vector>

struct SDL_Rect;

namespace AM
{
struct Camera;
class Tile;

namespace Client
{
class TileMap;
class SpriteData;
class UserInterface;

/**
 * This class is responsible for figuring out which sprites from the world are
 * relevant to the current frame and providing them in a sorted, prepared form.
 */
class WorldSpritePreparer
{
public:
    WorldSpritePreparer(entt::registry& inRegistry, const TileMap& inTileMap,
                        const SpriteData& inSpriteData,
                        const UserInterface& inUI);

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
     * Performs the tile portion of the gather step.
     */
    void gatherTileSpriteInfo(const Camera& camera, double alpha);

    // All of these just call pushTileSprite(), but Floor and Wall also check 
    // if the UI wants to swap any of their sprites with a phantom.
    void pushFloorSprite(const Tile& tile, const Camera& camera, int x, int y);
    void pushFloorCoveringSprites(const Tile& tile, const Camera& camera, int x,
                                 int y);
    void pushWallSprites(const Tile& tile, const Camera& camera, int x,
                         int y);
    void pushObjectSprites(const Tile& tile, const Camera& camera, int x,
                           int y);

    /**
     * Pushes the given tile sprite into the appropriate vector, based on 
     * whether it needs to be sorted or not.
     */
    void pushTileSprite(const Sprite& sprite, const Camera& camera,
                        int x, int y, TileLayer::Type layerType);

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

    /** Used for getting sprite color mods and phantom sprites. */
    const UserInterface& ui;

    /** Stores a temporary copy of the UI's desired phantom tile sprites. 
        We make a copy so that we can remove them as they get used. */
    std::vector<PhantomTileSpriteInfo> phantomTileSprites;

    /** Stores a temporary copy of the UI's desired tile sprite color mods.
        We make a copy so that we can remove them as they get used. */
    std::vector<TileSpriteColorModInfo> tileSpriteColorMods;

    /** Holds floor sprites during the gather step before they get pushed into 
        sortedSprites. */
    std::vector<SpriteSortInfo> floorSprites;

    /** Holds floor covering sprites during the gather step before they get 
        pushed into sortedSprites. */
    std::vector<SpriteSortInfo> floorCoveringSprites;

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
