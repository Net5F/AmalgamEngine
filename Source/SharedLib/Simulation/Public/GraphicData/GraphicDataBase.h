#pragma once

#include "Sprite.h"
#include "Animation.h"
#include "GraphicRef.h"
#include "GraphicSets.h"
#include "GraphicID.h"
#include "nlohmann/json_fwd.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>

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
     * Errors in debug if the given ID doesn't exist, returns the null sprite 
     * in release.
     */
    const Sprite& getSprite(std::string_view stringID) const;

    /**
     * Returns the sprite with the given numeric ID.
     * Errors in debug if the given ID doesn't exist, returns the null sprite 
     * in release.
     */
    const Sprite& getSprite(SpriteID numericID) const;

    /**
     * Returns the animation with the given string ID.
     * Errors in debug if the given ID doesn't exist, returns the null 
     * animation in release.
     */
    const Animation& getAnimation(std::string_view stringID) const;

    /**
     * Returns the animation with the given numeric ID.
     * Errors in debug if the given ID doesn't exist, returns the null 
     * animation in release.
     */
    const Animation& getAnimation(AnimationID numericID) const;

    /**
     * Returns the graphic with the given numeric ID.
     * Errors in debug if the given ID doesn't exist, returns the null 
     * graphic in release.
     */
    GraphicRef getGraphic(GraphicID numericID) const;

    /**
     * Returns the graphic set with the given string ID.
     * Errors in debug if the given ID doesn't exist, returns the null set 
     * in release.
     */
    const TerrainGraphicSet& getTerrainGraphicSet(std::string_view stringID);
    const FloorGraphicSet& getFloorGraphicSet(std::string_view stringID);
    const WallGraphicSet& getWallGraphicSet(std::string_view stringID);
    const ObjectGraphicSet& getObjectGraphicSet(std::string_view stringID);
    const EntityGraphicSet& getEntityGraphicSet(std::string_view stringID);

    /**
     * Returns the sprite set with the given numeric ID.
     * Errors if the given ID doesn't exist.
     */
    const TerrainGraphicSet&
        getTerrainGraphicSet(TerrainGraphicSetID numericID) const;
    const FloorGraphicSet& getFloorGraphicSet(FloorGraphicSetID numericID) const;
    const WallGraphicSet& getWallGraphicSet(WallGraphicSetID numericID) const;
    const ObjectGraphicSet&
        getObjectGraphicSet(ObjectGraphicSetID numericID) const;
    const EntityGraphicSet&
        getEntityGraphicSet(EntityGraphicSetID numericID) const;

    /**
     * Returns a reference to a vector containing all the sprites.
     */
    const std::vector<Sprite>& getAllSprites() const;

    /**
     * Returns a reference to a vector containing all the graphic sets of a
     * particular type.
     */
    const std::vector<TerrainGraphicSet>& getAllTerrainGraphicSets() const;
    const std::vector<FloorGraphicSet>& getAllFloorGraphicSets() const;
    const std::vector<WallGraphicSet>& getAllWallGraphicSets() const;
    const std::vector<ObjectGraphicSet>& getAllObjectGraphicSets() const;
    const std::vector<EntityGraphicSet>& getAllEntityGraphicSets() const;

protected:
    /**
     * Parses the given json, constructing sprites and pushing them into the
     * sprites vector.
     *
     * @param json The json to parse. Must be loaded from a valid
     *             ResourceData.json.
     */
    void parseJson(const nlohmann::json& json);

    /**
     * Resizes the vectors to fit the graphics and graphic sets in the given 
     * json.
     */
    void resizeVectors(const nlohmann::json& json);

    /**
     * Parses the given sprite json and adds the resulting sprite to the
     * sprites vector.

     * @param spriteJson The json to parse. Must be a valid sprite section
     *                   from ResourceData.json.
     */
    void parseSprite(const nlohmann::json& spriteJson);

    /**
     * Parses the given animation json and adds the resulting animation to the
     * animations vector.

     * @param animationJson The json to parse. Must be a valid animation section
     *                      from ResourceData.json.
     */
    void parseAnimation(const nlohmann::json& animationJson);

    /**
     * Parses the given graphic set json and adds the resulting graphic set to
     * the appropriate vector.
     *
     * @param graphicSetJson The json to parse. Must be a valid graphic set
     *                       section from ResourceData.json, for the appropriate
     *                       set type.
     */
    void parseTerrainGraphicSet(const nlohmann::json& graphicSetJson);
    void parseFloorGraphicSet(const nlohmann::json& graphicSetJson);
    void parseWallGraphicSet(const nlohmann::json& graphicSetJson);
    void parseObjectGraphicSet(const nlohmann::json& graphicSetJson);
    void parseEntityGraphicSet(const nlohmann::json& graphicSetJson);

    /** The loaded sprites, indexed by their numeric IDs. */
    std::vector<Sprite> sprites;

    /** The loaded animations, indexed by their numeric IDs. */
    std::vector<Animation> animations;

    /** The loaded graphic sets of each type. These are indexed by the set
        type's numeric IDs. (Each sprite set type has a unique ID space.) */
    std::vector<TerrainGraphicSet> terrainGraphicSets;
    std::vector<FloorGraphicSet> floorGraphicSets;
    std::vector<WallGraphicSet> wallGraphicSets;
    std::vector<ObjectGraphicSet> objectGraphicSets;
    std::vector<EntityGraphicSet> entityGraphicSets;

    /** A map for easily looking up sprites by their string ID. */
    std::unordered_map<std::string, const Sprite*> spriteStringMap;

    /** A map for easily looking up animations by their string ID. */
    std::unordered_map<std::string, const Animation*> animationStringMap;

    /** Maps for easily looking up graphic sets by their string ID. */
    std::unordered_map<std::string, const TerrainGraphicSet*>
        terrainGraphicSetStringMap;
    std::unordered_map<std::string, const FloorGraphicSet*>
        floorGraphicSetStringMap;
    std::unordered_map<std::string, const WallGraphicSet*>
        wallGraphicSetStringMap;
    std::unordered_map<std::string, const ObjectGraphicSet*>
        objectGraphicSetStringMap;
    std::unordered_map<std::string, const EntityGraphicSet*>
        entityGraphicSetStringMap;
};

} // End namespace AM
