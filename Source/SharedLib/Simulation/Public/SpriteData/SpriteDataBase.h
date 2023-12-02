#pragma once

#include "Sprite.h"
#include "SpriteSets.h"
#include <vector>
#include <unordered_map>
#include "nlohmann/json_fwd.hpp"

namespace AM
{

/**
 * Base class for Client::SpriteData and Server::SpriteData.
 * Loads the shared sprite data from ResourceData.json into memory and provides
 * an interface for accessing it.
 *
 * Also adds the "Null" sprite, for use as a default.
 *
 * The data in this class is immutable. To modify it, edit ResourceData.json
 * using the ResourceImporter.
 */
class SpriteDataBase
{
public:
    /**
     * Attempts to parse ResourceData.json and load all of the sprite data.
     *
     * Errors if resourceDataJson doesn't contain a spriteSheets section.
     */
    SpriteDataBase(const nlohmann::json& resourceDataJson);

    /**
     * Returns the sprite with the given string ID.
     * Errors if the given ID doesn't exist.
     */
    const Sprite& getSprite(const std::string& stringID) const;

    /**
     * Returns the sprite with the given numeric ID.
     * Errors if the given ID doesn't exist.
     */
    const Sprite& getSprite(int numericID) const;

    /**
     * Returns the sprite set with the given string ID.
     * Errors if the given ID doesn't exist.
     */
    const FloorSpriteSet& getFloorSpriteSet(const std::string& stringID) const;
    const FloorCoveringSpriteSet&
        getFloorCoveringSpriteSet(const std::string& stringID) const;
    const WallSpriteSet& getWallSpriteSet(const std::string& stringID) const;
    const ObjectSpriteSet&
        getObjectSpriteSet(const std::string& stringID) const;

    /**
     * Returns the sprite set with the given numeric ID.
     * Errors if the given ID doesn't exist.
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
    /** The index of the null sprite. Will always be the last sprite in the
        vector.
        Note: Don't get confused, the null sprite's ID is -1, but its index 
              is at the end of the vector. We do this instead of putting it at
              index 0, because we would have to save it in ResourceData.json and 
              add logic to skip it in the editor. */
    int nullSpriteIndex;

private:
    /**
     * Parses the given json, constructing sprites and pushing them into the
     * sprites vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              ResourceData.json.
     */
    void parseJson(const nlohmann::json& json);

    /**
     * Parses the given sprite json and adds the resulting sprite to the
     * sprites vector.

     * @param spriteJson  The json to parse. Must be a valid sprite section
     *                    from ResourceData.json.
     */
    void parseSprite(const nlohmann::json& spriteJson);

    /**
     * Parses the given sprite set json and adds the resulting sprite set to 
     * the appropriate vector.
     *
     * @param spriteSetJson  The json to parse. Must be a valid sprite set 
     *                       section from ResourceData.json, for the appropriate 
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
