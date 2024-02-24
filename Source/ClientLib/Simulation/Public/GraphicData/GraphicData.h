#pragma once

#include "GraphicDataBase.h"
#include "SpriteRenderData.h"
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
    const SpriteRenderData& getRenderData(SpriteID numericID) const;

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
     * renderData vector.
     *
     * @param spriteJson  The json to parse. Must be a valid sprite section
     *                    from ResourceData.json.
     * @param spriteSheetRelPath  The relative path to the sprite sheet image
     *                            file that holds this sprite.
     * @param texture  The texture of the spritesheet that contains the sprite.
     */
    void parseSprite(const nlohmann::json& spriteJson,
                     const std::string& spriteSheetRelPath,
                     const TextureHandle& texture);

    /** The loaded rendering data for each sprite, indexed by their numeric
        IDs */
    std::vector<SpriteRenderData> renderData;
};

} // End namespace Client
} // End namespace AM
