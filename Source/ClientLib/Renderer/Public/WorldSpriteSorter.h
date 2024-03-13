#pragma once

#include "SpriteSortInfo.h"
#include "TileLayers.h"
#include "PhantomSpriteInfo.h"
#include "SpriteColorModInfo.h"
#include "Timer.h"
#include "entt/entity/registry.hpp"
#include <vector>

struct SDL_Rect;

namespace AM
{
struct Camera;
class Tile;
struct GraphicState;

namespace Client
{
class TileMap;
class GraphicData;
class UserInterface;
struct AnimationState;

/**
 * Gathers all of the World's entity and tile layer sprites and sorts them
 * into their proper draw order.
 */
class WorldSpriteSorter
{
public:
    WorldSpriteSorter(entt::registry& inRegistry, const TileMap& inTileMap,
                      const GraphicData& inGraphicData,
                      const UserInterface& inUI);

    /**
     * Clears the stored sprite info, gathers the updated batch of sprites,
     * and sorts them into their proper draw order.
     */
    void sortSprites(const Camera& camera, double alpha);

    /**
     * @return A reference to the vector of sprites that was built during the
     *         last sortSprites() call.
     *
     * Note: Some of the returned vector's elements may be pure phantoms.
     *       Phantoms will have an empty spriteOwnerID field.
     */
    const std::vector<SpriteSortInfo>& getSortedSprites();

private:
    /**
     * Gathers the sprites that should be drawn on this frame, calculates their
     * screen position and bounding box for this frame, and pushes their data
     * into the sprites vector.
     *
     * @param camera  The camera to calculate screen position with.
     * @param alpha  For entities, the alpha to lerp between positions with.
     *
     * @post sortedSprites is filled with the floor sprites, and spritesToSort
     *       is filled with the sprites that need to be sorted.
     */
    void gatherSpriteInfo(const Camera& camera, double alpha);

    /**
     * Performs the tile portion of the gather step.
     */
    void gatherTileSpriteInfo(const Camera& camera);

    /**
     * Performs the entity portion of the gather step.
     */
    void gatherEntitySpriteInfo(const Camera& camera, double alpha);

    // All of these just call pushTileSprite(), but Floor and Wall also check
    // if the UI wants to swap any of their sprites with a phantom.
    void pushFloorSprite(const Tile& tile, const Camera& camera, int x, int y);
    void pushFloorCoveringSprites(const Tile& tile, const Camera& camera, int x,
                                  int y);
    void pushWallSprites(const Tile& tile, const Camera& camera, int x, int y);
    void pushObjectSprites(const Tile& tile, const Camera& camera, int x,
                           int y);

    /**
     * Pushes the given tile graphic into the appropriate vector, based on
     * whether it needs to be sorted or not.
     */
    void pushTileSprite(const GraphicRef& graphic, const Camera& camera,
                        const TileLayerID& layerID, bool isFullPhantom);

    /**
     * Returns the current sprite for the given entity data, or nullptr if 
     * the entity is using an empty animation.
     * Updates animationState's data if necessary.
     */
    const Sprite& getEntitySprite(const GraphicState& graphicState,
                                  AnimationState& animationState);

    /**
     * Pushes the given entity sprite into the sorting vector.
     */
    void pushEntitySprite(entt::entity entity, const Position& position,
                          const Sprite& sprite, const Camera& camera);

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

    /**
     * Returns a color mod from spriteColorMods for the given world object ID
     * if one exists, else returns {255, 255, 255, 255}.
     * If a color mod is returned, it will be removed from spriteColorMods.
     */
    template<typename T>
    SDL_Color getColorMod(const T& objectID);

    /** Used for gathering sprites. */
    entt::registry& registry;

    /** Used for gathering tiles. */
    const TileMap& tileMap;

    /** Used for getting graphic render data. */
    const GraphicData& graphicData;

    /** Used for getting sprite color mods and phantom sprites. */
    const UserInterface& ui;

    /** Stores a temporary copy of the UI's desired phantom sprites.
        We make a copy so that we can remove them as they get used. */
    std::vector<PhantomSpriteInfo> phantomSprites;

    /** Stores a temporary copy of the UI's desired sprite color mods.
        We make a copy so that we can remove them as they get used. */
    std::vector<SpriteColorModInfo> spriteColorMods;

    /** Holds floor sprites during the gather step before they get pushed into
        sortedSprites. */
    std::vector<SpriteSortInfo> floorSprites;

    /** Holds floor covering sprites during the gather step before they get
        pushed into sortedSprites. */
    std::vector<SpriteSortInfo> floorCoveringSprites;

    /** Stores the sorted sprite info from the last prepareSprites() call.
        Calculations and sorting are done in-place. */
    std::vector<SpriteSortInfo> sortedSprites;

    /** Holds sprites that need to be sorted. Sprites are pushed during
        gatherSpriteInfo() and sorted during sortSpritesByDepth(). */
    std::vector<SpriteSortInfo> spritesToSort;

    /** The timer used for animations. Having a single timer allows us to 
        keep animations in sync if we care to (which we do for tiles), or to 
        just use timestamps to know which frame of an animation to display. */
    Timer animationTimer;

    /** The timestamp from animationTimer that we're using during the current 
        render frame. */
    double lastAnimationTimestamp;

    /** The timestamp from animationTimer that we used during the last render 
        frame. Allows us to calculate time deltas. */
    double currentAnimationTimestamp;
};

template<typename T>
SDL_Color WorldSpriteSorter::getColorMod(const T& objectID)
{
    auto objectIDsMatch = [&](const SpriteColorModInfo& info) {
        // If this color mod is for the same type of object.
        if (const T* colorModLayerID = std::get_if<T>(&(info.objectToModify))) {
            // If the IDs match, return true.
            if (*colorModLayerID == objectID) {
                return true;
            }
        }

        return false;
    };

    // If the UI wants a color mod on this sprite, use it.
    auto colorModInfo = std::find_if(spriteColorMods.begin(),
                                     spriteColorMods.end(), objectIDsMatch);
    if (colorModInfo != spriteColorMods.end()) {
        // Remove this color mod from our temp vector, since it's been used.
        SDL_Color colorMod{colorModInfo->colorMod};
        spriteColorMods.erase(colorModInfo);
        return colorMod;
    }
    else {
        return {0, 0, 0, 255};
    }
}

} // End namespace Client
} // End namespace AM
