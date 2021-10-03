#pragma once

#include "Sprite.h"
#include <vector>
#include <unordered_map>
#include "nlohmann/json_fwd.hpp"

namespace AM
{
namespace Server
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
     * Errors if SpriteData.json doesn't exist or it fails to parse.
     */
    SpriteData();

    /**
     * Get a sprite, using its string ID.
     */
    const Sprite& get(const std::string& stringID);

    /**
     * Get a sprite, using its numeric ID.
     */
    const Sprite& get(int numericID);

private:
    /**
     * Parses the given sprite json and adds the resulting sprite to the
     * sprites vector.
     *
     * @param spriteJson  The json to parse. Must be a valid sprite section
     *                    from SpriteData.json.
     */
    void parseSprite(const nlohmann::json& spriteJson);

    /**
     * Parses the given json, constructing sprites and pushing them into the
     * sprites vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              SpriteData.json.
     */
    void parseJson(nlohmann::json& json);

    /** The loaded sprite data. */
    std::vector<Sprite> sprites;

    /** A map for easily looking up sprites by their string ID. */
    std::unordered_map<std::string, const Sprite*> stringMap;

    /** A map for easily looking up sprites by their numeric ID. */
    std::unordered_map<int, const Sprite*> numericMap;
};

} // End namespace Server
} // End namespace AM
