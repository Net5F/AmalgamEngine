#pragma once

#include "SpriteSortInfo.h"
#include "PhantomSpriteInfo.h"
#include "SpriteColorModInfo.h"
#include "EntityGraphicType.h"
#include "GraphicID.h"
#include <vector>

struct SDL_FRect;

namespace AM
{
struct Camera;
class Tile;
struct TileLayerID;
struct GraphicState;
struct AudioVisualEffectInstance;

namespace Client
{
class World;
class GraphicData;
class UserInterface;
struct ClientGraphicState;

/**
 * Gathers all of the World's entity and tile layer sprites and sorts them
 * into their proper draw order.
 */
class WorldSpriteSorter
{
public:
    WorldSpriteSorter(World& inWorld, const GraphicData& inGraphicData,
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

    /**
     * Performs the audio/visual effect portion of the gather step.
     */
    void gatherAVEffectSpriteInfo(const Camera& camera, double alpha);

    // All of these just call pushTileSprite(), but Floor and Wall also check
    // if the UI wants to swap any of their sprites with a phantom.
    void pushTerrainSprites(const Tile& tile, const Camera& camera,
                            const TilePosition& tilePosition);
    void pushFloorSprite(const Tile& tile, const Camera& camera,
                         const TilePosition& tilePosition);
    void pushWallSprites(const Tile& tile, const Camera& camera,
                         const TilePosition& tilePosition);
    void pushObjectSprites(const Tile& tile, const Camera& camera,
                           const TilePosition& tilePosition);

    /**
     * Pushes the given tile graphic into the appropriate vector, based on
     * whether it needs to be sorted or not.
     */
    void pushTileSprite(const GraphicRef& graphic, const Camera& camera,
                        const TileLayerID& layerID, bool isFullPhantom);

    /**
     * Returns the current sprite for the given entity data, or nullptr if 
     * the entity is using an empty animation.
     * Updates clientGraphicState's data if necessary.
     */
    const Sprite& getEntitySprite(const GraphicState& graphicState,
                                  ClientGraphicState& clientGraphicState);

    /**
     * Pushes the given entity sprite into the sorting vector.
     */
    void pushEntitySprite(entt::entity entity, const Position& position,
                          const Sprite& sprite, const Camera& camera,
                          EntityGraphicSetID graphicSetID,
                          EntityGraphicType graphicType);

    /**
     * Returns the current sprite for the given AV effect data, or nullptr if 
     * the effect is using an empty animation.
     * Updates the effect's animation state data if necessary.
     */
    const Sprite& getAVEffectSprite(AudioVisualEffectInstance& instance,
                                    GraphicID graphicID);

    /**
     * Pushes the given AV effect sprite into the sorting vector.
     */
    void pushAVEffectSprite(AudioVisualEffectInstance& instance,
                            const Position& position, const Sprite& sprite,
                            const Camera& camera);

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
    bool isWithinScreenBounds(const SDL_FRect& extent, const Camera& camera);

    /**
     * Returns an appropriate graphic for the given phantom info.
     */
    GraphicRef getPhantomGraphic(const PhantomSpriteInfo& phantomSpriteInfo);

    /**
     * Returns a color mod from spriteColorMods for the given world object ID
     * if one exists, else returns {255, 255, 255, 255}.
     * If a color mod is returned, it will be removed from spriteColorMods.
     */
    template<typename T>
    SDL_Color getColorMod(const T& objectID);

    /**
     * Returns the height of the terrain at the given position, in world units.
     */
    Uint8 getTerrainHeight(const TilePosition& tilePosition);

    /** Used for gathering graphics. */
    World& world;

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

    /** Stores the sorted sprite info from the last prepareSprites() call.
        Calculations and sorting are done in-place. */
    std::vector<SpriteSortInfo> sortedSprites;

    /** Holds sprites that need to be sorted. Sprites are pushed during
        gatherSpriteInfo() and sorted during sortSpritesByDepth(). */
    std::vector<SpriteSortInfo> spritesToSort;

    /** The global timestamp that we're using during the current render 
        frame. */
    double lastAnimationTimestamp;

    /** The global timestamp that we used during the last render frame. 
        Allows us to calculate time deltas. */
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
