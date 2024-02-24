#pragma once

#include "Sprite.h"
#include "GraphicRef.h"
#include "GraphicSets.h"
#include "GraphicID.h"
#include <vector>
#include <unordered_map>
#include "nlohmann/json_fwd.hpp"

namespace AM
{

/**
 * Base class for Client::GraphicData and Server::GraphicData.
 * Loads the shared graphics data from ResourceData.json into memory and 
 * provides an interface for accessing it.
 *
 * The data in this class is immutable. To modify it, edit ResourceData.json
 * using the ResourceImporter.
 */
class GraphicDataBase
{
public:
    /**
     * Attempts to parse resourceDataJson and load all of the sprite data.
     *
     * Errors if resourceDataJson doesn't contain a spriteSheets section.
     */
    GraphicDataBase(const nlohmann::json& resourceDataJson);

    /**
     * Returns the sprite with the given string ID.
     * Errors in debug if the given ID doesn't exist.
     */
    const Sprite& getSprite(const std::string& stringID) const;

    /**
     * Returns the sprite with the given numeric ID.
     * Errors in debug if the given ID doesn't exist.
     */
    const Sprite& getSprite(SpriteID numericID) const;

    /**
     * Returns the graphic with the given numeric ID.
     * Errors in debug if the given ID doesn't exist.
     */
    GraphicRef getGraphic(GraphicID numericID) const;

    /**
     * Returns the graphic set with the given string ID.
     * Errors if the given ID doesn't exist.
     */
    const FloorGraphicSet& getFloorGraphicSet(const std::string& stringID) const;
    const FloorCoveringGraphicSet&
        getFloorCoveringGraphicSet(const std::string& stringID) const;
    const WallGraphicSet& getWallGraphicSet(const std::string& stringID) const;
    const ObjectGraphicSet&
        getObjectGraphicSet(const std::string& stringID) const;

    /**
     * Returns the sprite set with the given numeric ID.
     * Errors if the given ID doesn't exist.
     */
    const FloorGraphicSet& getFloorGraphicSet(FloorGraphicSetID numericID) const;
    const FloorCoveringGraphicSet&
        getFloorCoveringGraphicSet(FloorCoveringGraphicSetID numericID) const;
    const WallGraphicSet& getWallGraphicSet(WallGraphicSetID numericID) const;
    const ObjectGraphicSet&
        getObjectGraphicSet(ObjectGraphicSetID numericID) const;

    /**
     * Get a reference to a vector containing all the sprites.
     */
    const std::vector<Sprite>& getAllSprites() const;

    /**
     * Get a reference to a vector containing all the graphic sets of a
     * particular type.
     */
    const std::vector<FloorGraphicSet>& getAllFloorGraphicSets() const;
    const std::vector<FloorCoveringGraphicSet>&
        getAllFloorCoveringGraphicSets() const;
    const std::vector<WallGraphicSet>& getAllWallGraphicSets() const;
    const std::vector<ObjectGraphicSet>& getAllObjectGraphicSets() const;

protected:
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
     * Parses the given graphic set json and adds the resulting graphic set to
     * the appropriate vector.
     *
     * @param graphicSetJson  The json to parse. Must be a valid graphic set
     *                        section from ResourceData.json, for the appropriate
     *                        set type.
     */
    void parseFloorGraphicSet(const nlohmann::json& graphicSetJson);
    void parseFloorCoveringGraphicSet(const nlohmann::json& graphicSetJson);
    void parseWallGraphicSet(const nlohmann::json& graphicSetJson);
    void parseObjectGraphicSet(const nlohmann::json& graphicSetJson);

    /** The loaded sprites, indexed by their numeric IDs. */
    std::vector<Sprite> sprites;

    /** The loaded graphic sets of each type. These are indexed by the set
        type's numeric IDs. (Each sprite set type has a unique ID space.) */
    std::vector<FloorGraphicSet> floorGraphicSets;
    std::vector<FloorCoveringGraphicSet> floorCoveringGraphicSets;
    std::vector<WallGraphicSet> wallGraphicSets;
    std::vector<ObjectGraphicSet> objectGraphicSets;

    /** A map for easily looking up sprites by their string ID. */
    std::unordered_map<std::string, const Sprite*> spriteStringMap;

    /** Maps for easily looking up graphic sets by their string ID. */
    std::unordered_map<std::string, const FloorGraphicSet*>
        floorGraphicSetStringMap;
    std::unordered_map<std::string, const FloorCoveringGraphicSet*>
        floorCoveringGraphicSetStringMap;
    std::unordered_map<std::string, const WallGraphicSet*>
        wallGraphicSetStringMap;
    std::unordered_map<std::string, const ObjectGraphicSet*>
        objectGraphicSetStringMap;
};

} // End namespace AM
