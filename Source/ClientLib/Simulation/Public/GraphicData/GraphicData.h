#pragma once

#include "GraphicDataBase.h"
#include "SpriteRenderData.h"
#include "AnimationRenderData.h"
#include <vector>

namespace AM
{
class AssetCache;

namespace Client
{
/**
 * See GraphicDataBase class comment.
 *
 * Beyond the parsing that GraphicDataBase performs, this class loads the
 * rendering-specific graphics data and provides an interface for accessing it.
 *
 * The data in this class is immutable. To modify it, edit ResourceData.json
 * using the ResourceImporter.
 */
class GraphicData : public GraphicDataBase
{
public:
    /**
     * Calls GraphicDataBase() constructor, then load the rendering-specific
     * graphics data from the given json.
     *
     * Errors if resourceDataJson doesn't contain a spriteSheets section.
     */
    GraphicData(const nlohmann::json& resourceDataJson, AssetCache& assetCache);

    /**
     * Get a sprite's render data, using its numeric ID.
     */
    const SpriteRenderData& getSpriteRenderData(SpriteID numericID) const;

    /**
     * Get an animation's render data, using its numeric ID.
     */
    const AnimationRenderData&
        getAnimationRenderData(AnimationID numericID) const;

    /**
     * Returns an appropriate world-space offset to use when rendering the 
     * given graphic from the given set.
     *
     * If the graphic is an Animation, its alignment offset will be returned.
     * If the graphic is a Sprite, {0, 0, 0} will be returned (Sprites don't 
     * support alignment offsets).
     *
     * Note: Alignment offsets aren't used by the sim. Instead, it just centers 
     *       IdleSouth's modelBounds on the entity's Position. The renderer 
     *       then needs to do some work to make sure the sprite lines up.
     */
    Vector3 getRenderAlignmentOffset(EntityGraphicSetID setID,
                                     EntityGraphicType graphicType,
                                     Rotation::Direction direction) const;

private:
    /**
     * Parses the given json, constructing graphics and pushing them into the
     * renderData vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              ResourceData.json.
     * @param assetCache  The asset cache to load sprite sheet textures into.
     */
    void parseJson(const nlohmann::json& json, AssetCache& assetCache);

    /**
     * Parses the given sprite json and adds the resulting sprite to the
     * spriteRenderData vector.
     *
     * @param spriteJson The json to parse. Must be a valid sprite section
     *                   from ResourceData.json.
     * @param spriteSheetRelPath The relative path to the sprite sheet image
     *                           file that holds this sprite.
     * @param texture The texture of the spritesheet that contains the sprite.
     */
    void parseSprite(const nlohmann::json& spriteJson,
                     const std::string& spriteSheetRelPath,
                     const TextureHandle& texture);

    /**
     * Parses the given animation json and adds the resulting animation to the
     * animationRenderData vector.
     *
     * @param animationJson The json to parse. Must be a valid animation section
     *                      from ResourceData.json.
     */
    void parseAnimation(const nlohmann::json& animationJson);

    /** The loaded rendering data for each sprite, indexed by their numeric
        IDs */
    std::vector<SpriteRenderData> spriteRenderData;

    /** The loaded rendering data for each animation, indexed by their numeric
        IDs */
    std::vector<AnimationRenderData> animationRenderData;
};

} // End namespace Client
} // End namespace AM
