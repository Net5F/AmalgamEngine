#pragma once

#include "SpriteDataBase.h"
#include "SpriteRenderData.h"
#include "nlohmann/json_fwd.hpp"
#include <vector>

namespace AM
{
class AssetCache;

namespace Client
{
/**
 * See SpriteDataBase class comment.
 *
 * Beyond the parsing that SpriteDataBase performs, this class loads the 
 * rendering-specific sprite data and provides an interface for accessing it.
 *
 * Note: This class expects a SpriteData.json file to be present in the same
 *       directory as the application executable.
 *
 * The data in this class is immutable. To modify it, edit SpriteData.json 
 * using the SpriteEditor.
 */
class SpriteData : public SpriteDataBase
{
public:
    /**
     * Calls SpriteDataBase() constructor, then re-parses SpriteData.json to 
     * load the rendering-specific sprite data.
     *
     * Errors if SpriteData.json doesn't exist or it fails to parse.
     */
    SpriteData(AssetCache& assetCache);

    /**
     * Get a sprite's render data, using its numeric ID.
     */
    const SpriteRenderData& getRenderData(int numericID) const;

private:
    /**
     * Parses the given json, constructing sprites and pushing them into the
     * sprites vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              SpriteData.json.
     * @param assetCache  The asset cache to load sprite sheet textures into.
     */
    void parseJson(nlohmann::json& json, AssetCache& assetCache);

    /**
     * Parses the given sprite json and adds the resulting sprite to the
     * sprites vector.
     *
     * @param spriteJson  The json to parse. Must be a valid sprite section
     *                    from SpriteData.json.
     * @param texture  The texture of the spritesheet that contains the sprite.
     */
    void parseSprite(const nlohmann::json& spriteJson,
                     const TextureHandle& texture);

    /** The loaded rendering data for each sprite, indexed by their numeric 
        IDs */
    std::vector<SpriteRenderData> renderData;
};

} // End namespace Client
} // End namespace AM
