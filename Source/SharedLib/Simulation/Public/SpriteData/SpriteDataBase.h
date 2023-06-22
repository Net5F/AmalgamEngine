#pragma once

#include "Sprite.h"
#include "SpriteSets.h"
#include <vector>
#include <unordered_map>
#include "SDL_stdinc.h"
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
    const Sprite& getSprite(const std::string& stringID) const;

    /**
     * Get a sprite, using its numeric ID.
     */
    const Sprite& getSprite(int numericID) const;

    /**
     * Get a sprite set, using its string ID.
     */
    const FloorSpriteSet& getFloorSpriteSet(const std::string& stringID) const;
    const FloorCoveringSpriteSet&
        getFloorCoveringSpriteSet(const std::string& stringID) const;
    const WallSpriteSet& getWallSpriteSet(const std::string& stringID) const;
    const ObjectSpriteSet&
        getObjectSpriteSet(const std::string& stringID) const;

    /**
     * Get a sprite set, using its numeric ID.
     */
    const FloorSpriteSet& getFloorSpriteSet(Uint16 numericID) const;
    const FloorCoveringSpriteSet&
        getFloorCoveringSpriteSet(Uint16 numericID) const;
    const WallSpriteSet& getWallSpriteSet(Uint16 numericID) const;
    const ObjectSpriteSet& getObjectSpriteSet(Uint16 numericID) const;

    /**
     * Get a reference to a vector containing all the sprites.
     */
    const std::vector<Sprite>& getAllSprites() const;

    /**
     * Get a reference to a vector containing all the sprite sets of a 
     * particular type.
     */
    const std::vector<FloorSpriteSet>& getAllFloorSpriteSets() const;
    const std::vector<FloorCoveringSpriteSet>&
        getAllFloorCoveringSpriteSets() const;
    const std::vector<WallSpriteSet>& getAllWallSpriteSets() const;
    const std::vector<ObjectSpriteSet>& getAllObjectSpriteSets() const;

protected:
    /** The index of the empty sprite. Will always be the last sprite in the
        vector.
        Note: Don't get confused, the empty sprite's ID is -1, but its index 
              is at the end of the vector. We do this instead of putting it at
              index 0, because we would have to save it in SpriteData.json and 
              add logic to skip it in the editor. */
    int emptySpriteIndex;

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

     * @param spriteJson  The json to parse. Must be a valid sprite section
     *                    from SpriteData.json.
     */
    void parseSprite(const nlohmann::json& spriteJson);

    /**
     * Parses the given sprite set json and adds the resulting sprite set to 
     * the appropriate vector.
     *
     * @param spriteSetJson  The json to parse. Must be a valid sprite set 
     *                       section from SpriteData.json, for the appropriate 
     *                       set type.
     */
    void parseFloorSpriteSet(const nlohmann::json& spriteSetJson);
    void parseFloorCoveringSpriteSet(const nlohmann::json& spriteSetJson);
    void parseWallSpriteSet(const nlohmann::json& spriteSetJson);
    void parseObjectSpriteSet(const nlohmann::json& spriteSetJson);

    /** The loaded sprites, indexed by their numeric IDs. */
    std::vector<Sprite> sprites;

    /** The loaded sprite sets of each type. These are indexed by the set 
        type's numeric IDs. (Each sprite set type has a unique ID space.) */
    std::vector<FloorSpriteSet> floorSpriteSets;
    std::vector<FloorCoveringSpriteSet> floorCoveringSpriteSets;
    std::vector<WallSpriteSet> wallSpriteSets;
    std::vector<ObjectSpriteSet> objectSpriteSets;

    /** A map for easily looking up sprites by their string ID. */
    std::unordered_map<std::string, const Sprite*> spriteStringMap;

    /** Maps for easily looking up sprite sets by their string ID. */
    std::unordered_map<std::string, const FloorSpriteSet*>
        floorSpriteSetStringMap;
    std::unordered_map<std::string, const FloorCoveringSpriteSet*>
        floorCoveringSpriteSetStringMap;
    std::unordered_map<std::string, const WallSpriteSet*>
        wallSpriteSetStringMap;
    std::unordered_map<std::string, const ObjectSpriteSet*>
        objectSpriteSetStringMap;
};

} // End namespace AM
