#pragma once

#include "Sprite.h"
#include <vector>
#include <unordered_map>
#include "nlohmann/json_fwd.hpp"

namespace AM
{
/**
 * Base class for Client::SpriteData and Server::SpriteData.
 * Loads the shared sprite data from SpriteData.json into memory and provides 
 * an interface for accessing it.
 *
 * Also adds the "Empty" sprite, for use as a default.
 *
 * Note: This class expects a SpriteData.json file to be present in the same
 *       directory as the application executable.
 *
 * The data in this class is immutable. To modify it, edit SpriteData.json 
 * using the SpriteEditor.
 */
class SpriteDataBase
{
public:
    /**
     * Attempts to parse SpriteData.json and load all of the sprite data.
     *
     * Errors if SpriteData.json doesn't exist or it fails to parse.
     */
    SpriteDataBase();

    /**
     * Get a sprite, using its string ID.
     */
    const Sprite& get(const std::string& stringID) const;

    /**
     * Get a sprite, using its numeric ID.
     */
    const Sprite& get(int numericID) const;

    /**
     * Get a reference to the vector of all the sprites.
     */
    const std::vector<Sprite>& getAllSprites() const;

    /**
     * Get a sprite's unique display name, using its numeric ID.
     */
    const std::string& getDisplayName(int numericID) const;

    /**
     * Get a sprite's string ID, using its numeric ID.
     * 
     * Unlike the numeric ID, the string ID will be consistent, and can be 
     * used for persistent state.
     */
    const std::string& getStringID(int numericID) const;

protected:
    /** The index of the empty sprite. Will always be the last sprite in the 
        vectors.
        Note: Don't get confused, the empty sprite's ID is -1, but its index 
              is at the end of the vectors. We do this instead of putting it 
              at index 0, because we would have to save it in SpriteData.json 
              and add logic to skip it in the editor. */
    unsigned int emptySpriteIndex;

private:
    /**
     * Parses the given json, constructing sprites and pushing them into the
     * sprites vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              SpriteData.json.
     */
    void parseJson(nlohmann::json& json);

    /**
     * Parses the given sprite json and adds the resulting sprite to the
     * sprites vector.
     *
     * @param spriteJson  The json to parse. Must be a valid sprite section
     *                    from SpriteData.json.
     */
    void parseSprite(const nlohmann::json& spriteJson);

    /**
     * Returns the number of sprites that are in the given SpriteData.json 
     * + 1 for the empty sprite.
     */
    unsigned int getSpriteCount(const nlohmann::json& json);

    /** The loaded sprites, indexed by their numeric IDs. */
    std::vector<Sprite> sprites;

    /** Each sprite's unique display name, indexed by their numeric ID. */
    std::vector<std::string> displayNames;

    /** Each sprite's string ID, indexed by their numeric ID.
        String IDs are derived from displayName by replacing spaces with 
        underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::vector<std::string> stringIDs;

    /** A map for easily looking up sprites by their string ID. */
    std::unordered_map<std::string, const Sprite*> stringMap;
};

} // End namespace AM
