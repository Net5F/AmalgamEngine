#pragma once

#include "Sprite.h"
#include <vector>
#include <unordered_map>
#include "nlohmann/json_fwd.hpp"

namespace AM
{
class AssetCache;

namespace Client
{
/**
 * Loads the sprite data from SpriteData.json into memory and provides an
 * interface for accessing it.
 *
 * Note: This class expects a SpriteData.json file to be present in the same
 *       directory as the application executable.
 *
 * The data in this class is immutable. To modify it, edit SpriteData.json in
 * the SpriteEditor.
 */
class SpriteData
{
public:
    /**
     * Attempts to parse SpriteData.json and construct all of the sprite
     * objects.
     *
     * Also adds an "Empty" sprite, for use as a default.
     *
     * Errors if SpriteData.json doesn't exist or it fails to parse.
     */
    SpriteData(AssetCache& assetCache);

    /**
     * Get a sprite, using its string ID.
     */
    const Sprite& get(const std::string& stringID);

    /**
     * Get a sprite, using its numeric ID.
     */
    const Sprite& get(int numericID);

    /**
     * Get a reference to a vector of all the sprites.
     */
    const std::vector<Sprite>& getAllSprites() const;

private:
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

    /**
     * Parses the given json, constructing sprites and pushing them into the
     * sprites vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              SpriteData.json.
     * @param assetCache  The asset cache to load sprite sheet textures into.
     */
    void parseJson(nlohmann::json& json, AssetCache& assetCache);

    /** The loaded sprite data. */
    std::vector<Sprite> sprites;

    /** A map for easily looking up sprites by their string ID. */
    std::unordered_map<std::string, const Sprite*> stringMap;

    /** A map for easily looking up sprites by their numeric ID. */
    std::unordered_map<int, const Sprite*> numericMap;
};

} // End namespace Client
} // End namespace AM
