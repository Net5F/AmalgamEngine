#include "GraphicDataBase.h"
#include "SpriteID.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"

namespace AM
{
GraphicDataBase::GraphicDataBase(const nlohmann::json& resourceDataJson)
: sprites{}
, floorGraphicSets{}
, floorCoveringGraphicSets{}
, wallGraphicSets{}
, objectGraphicSets{}
, spriteStringMap{}
, floorGraphicSetStringMap{}
, floorCoveringGraphicSetStringMap{}
, wallGraphicSetStringMap{}
, objectGraphicSetStringMap{}
{
    // Parse the json structure to construct our sprites and animations.
    parseJson(resourceDataJson);
}

const Sprite& GraphicDataBase::getSprite(const std::string& stringID) const
{
    // Attempt to find the given string ID.
    auto it{spriteStringMap.find(stringID)};
    if (it == spriteStringMap.end()) {
        LOG_ERROR("Failed to find sprite with string ID: %s", stringID.c_str());
        return sprites[0];
    }

    return *(it->second);
}

const Sprite& GraphicDataBase::getSprite(SpriteID numericID) const
{
    if (numericID >= sprites.size()) {
        LOG_ERROR("Invalid numeric ID while getting sprite: %d", numericID);
        return sprites[0];
    }

    return sprites[numericID];
}

GraphicRef GraphicDataBase::getGraphic(GraphicID numericID) const
{
    if (numericID >= sprites.size()) {
        LOG_ERROR("Invalid numeric ID while getting graphic: %d", numericID);
        return {sprites[0]};
    }

    //bool isAnimation{(static_cast<Uint32>(numericID) & GRAPHIC_ID_TYPE_MASK)
    //                 >> 31};
    //if (isAnimation) {
    //    AnimationID animationID{numericID & GRAPHIC_ID_VALUE_MASK};
    //    return getAnimation(animationID);
    //}
    //else {
        SpriteID spriteID{numericID & GRAPHIC_ID_VALUE_MASK};
        return {getSprite(spriteID)};
    //}
}

const FloorGraphicSet&
    GraphicDataBase::getFloorGraphicSet(const std::string& stringID) const
{
    auto it{floorGraphicSetStringMap.find(stringID)};
    if (it == floorGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  stringID.c_str());
        return floorGraphicSets[0];
    }

    return *(it->second);
}

const FloorCoveringGraphicSet&
    GraphicDataBase::getFloorCoveringGraphicSet(const std::string& stringID) const
{
    auto it{floorCoveringGraphicSetStringMap.find(stringID)};
    if (it == floorCoveringGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  stringID.c_str());
        return floorCoveringGraphicSets[0];
    }

    return *(it->second);
}

const WallGraphicSet&
    GraphicDataBase::getWallGraphicSet(const std::string& stringID) const
{
    auto it{wallGraphicSetStringMap.find(stringID)};
    if (it == wallGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  stringID.c_str());
        return wallGraphicSets[0];
    }

    return *(it->second);
}

const ObjectGraphicSet&
    GraphicDataBase::getObjectGraphicSet(const std::string& stringID) const
{
    auto it{objectGraphicSetStringMap.find(stringID)};
    if (it == objectGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  stringID.c_str());
        return objectGraphicSets[0];
    }

    return *(it->second);
}

const FloorGraphicSet&
    GraphicDataBase::getFloorGraphicSet(FloorGraphicSetID numericID) const
{
    if (numericID >= floorGraphicSets.size()) {
        LOG_ERROR("Invalid numeric ID while getting graphic set: %d", numericID);
        return floorGraphicSets[0];
    }

    return floorGraphicSets[numericID];
}

const FloorCoveringGraphicSet& GraphicDataBase::getFloorCoveringGraphicSet(
    FloorCoveringGraphicSetID numericID) const
{
    if (numericID >= floorCoveringGraphicSets.size()) {
        LOG_ERROR("Invalid numeric ID while getting graphic set: %d", numericID);
        return floorCoveringGraphicSets[0];
    }

    return floorCoveringGraphicSets[numericID];
}

const WallGraphicSet&
    GraphicDataBase::getWallGraphicSet(WallGraphicSetID numericID) const
{
    if (numericID >= wallGraphicSets.size()) {
        LOG_ERROR("Invalid numeric ID while getting graphic set: %d", numericID);
        return wallGraphicSets[0];
    }

    return wallGraphicSets[numericID];
}

const ObjectGraphicSet&
    GraphicDataBase::getObjectGraphicSet(ObjectGraphicSetID numericID) const
{
    if (numericID >= objectGraphicSets.size()) {
        LOG_ERROR("Invalid numeric ID while getting graphic set: %d", numericID);
        return objectGraphicSets[0];
    }

    return objectGraphicSets[numericID];
}

const std::vector<Sprite>& GraphicDataBase::getAllSprites() const
{
    return sprites;
}

const std::vector<FloorGraphicSet>& GraphicDataBase::getAllFloorGraphicSets() const
{
    return floorGraphicSets;
}

const std::vector<FloorCoveringGraphicSet>&
    GraphicDataBase::getAllFloorCoveringGraphicSets() const
{
    return floorCoveringGraphicSets;
}

const std::vector<WallGraphicSet>& GraphicDataBase::getAllWallGraphicSets() const
{
    return wallGraphicSets;
}

const std::vector<ObjectGraphicSet>&
    GraphicDataBase::getAllObjectGraphicSets() const
{
    return objectGraphicSets;
}

void GraphicDataBase::parseJson(const nlohmann::json& json)
{
    // Add the null sprite and graphic sets.
    GraphicRef nullSprite{sprites.emplace_back("Null", "null", NULL_SPRITE_ID)};
    floorGraphicSets.emplace_back(GraphicSet{"Null", "null"},
                                  NULL_FLOOR_GRAPHIC_SET_ID, nullSprite);
    floorCoveringGraphicSets.emplace_back(
        GraphicSet{"Null", "null"}, NULL_FLOOR_COVERING_GRAPHIC_SET_ID,
        std::array<GraphicRef, FloorCoveringGraphicSet::VARIATION_COUNT>{
            nullSprite, nullSprite, nullSprite, nullSprite, nullSprite,
            nullSprite, nullSprite, nullSprite});
    wallGraphicSets.emplace_back(
        GraphicSet{"Null", "null"}, NULL_WALL_GRAPHIC_SET_ID,
        std::array<GraphicRef, Wall::Type::Count>{
            nullSprite, nullSprite, nullSprite, nullSprite});
    objectGraphicSets.emplace_back(
        GraphicSet{"Null", "null"}, NULL_OBJECT_GRAPHIC_SET_ID,
        std::array<GraphicRef, ObjectGraphicSet::VARIATION_COUNT>{
            nullSprite, nullSprite, nullSprite, nullSprite, nullSprite,
            nullSprite, nullSprite, nullSprite});

    // Parse the json and catch any parsing errors.
    try {
        // Iterate every sprite sheet and add all of their sprites.
        for (auto& sheetJson : json.at("spriteSheets").items()) {
            for (auto& spriteJson : sheetJson.value().at("sprites").items()) {
                // Parse the sprite's data and add it to our containers.
                parseSprite(spriteJson.value());
            }
        }

        // Add each type of graphic set.
        for (auto& floorJson : json.at("floors").items()) {
            parseFloorGraphicSet(floorJson.value());
        }
        for (auto& floorCoveringJson : json.at("floorCoverings").items()) {
            parseFloorCoveringGraphicSet(floorCoveringJson.value());
        }
        for (auto& wallJson : json.at("walls").items()) {
            parseWallGraphicSet(wallJson.value());
        }
        for (auto& objectJson : json.at("objects").items()) {
            parseObjectGraphicSet(objectJson.value());
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL(
            "Failed to parse sprites and graphic sets in ResourceData.json: %s",
            e.what());
    }

    // Add everything to the associated maps.
    for (const Sprite& sprite : sprites) {
        spriteStringMap.emplace(sprite.stringID, &sprites[sprite.numericID]);
    }
    for (const FloorGraphicSet& set : floorGraphicSets) {
        floorGraphicSetStringMap.emplace(set.stringID,
                                        &floorGraphicSets[set.numericID]);
    }
    for (const FloorCoveringGraphicSet& set : floorCoveringGraphicSets) {
        floorCoveringGraphicSetStringMap.emplace(
            set.stringID, &floorCoveringGraphicSets[set.numericID]);
    }
    for (const WallGraphicSet& set : wallGraphicSets) {
        wallGraphicSetStringMap.emplace(set.stringID,
                                       &wallGraphicSets[set.numericID]);
    }
    for (const ObjectGraphicSet& set : objectGraphicSets) {
        objectGraphicSetStringMap.emplace(set.stringID,
                                         &objectGraphicSets[set.numericID]);
    }
}

void GraphicDataBase::parseSprite(const nlohmann::json& spriteJson)
{
    // Add the sprite to the sprites vector.
    Sprite& sprite{sprites.emplace_back()};

    // Add the display name and IDs.
    sprite.numericID = spriteJson.at("numericID");
    sprite.displayName = spriteJson.at("displayName").get<std::string>();
    sprite.stringID = spriteJson.at("stringID").get<std::string>();

    // Add whether the sprite has a bounding box or not.
    sprite.collisionEnabled = spriteJson.at("collisionEnabled");

    // Add the model-space bounds.
    sprite.modelBounds.minX = spriteJson.at("modelBounds").at("minX");
    sprite.modelBounds.maxX = spriteJson.at("modelBounds").at("maxX");
    sprite.modelBounds.minY = spriteJson.at("modelBounds").at("minY");
    sprite.modelBounds.maxY = spriteJson.at("modelBounds").at("maxY");
    sprite.modelBounds.minZ = spriteJson.at("modelBounds").at("minZ");
    sprite.modelBounds.maxZ = spriteJson.at("modelBounds").at("maxZ");
}

void GraphicDataBase::parseFloorGraphicSet(const nlohmann::json& graphicSetJson)
{
    FloorGraphicSetID numericID{graphicSetJson.at("numericID")};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};
    std::string stringID{graphicSetJson.at("stringID").get<std::string>()};

    // Add the graphics.
    // Note: Floors just have 1 sprite, but the json uses an array in case we
    //       want to add variations in the future.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};

    // Save the graphic set in the appropriate vector.
    floorGraphicSets.emplace_back(
        GraphicSet{displayName, stringID}, numericID,
        GraphicRef{getSprite(graphicIDJson[0].get<GraphicID>())});
}

void GraphicDataBase::parseFloorCoveringGraphicSet(
    const nlohmann::json& graphicSetJson)
{
    // Add a graphic set to the appropriate vector.
    GraphicRef nullSprite{sprites[0]};
    FloorCoveringGraphicSet& graphicSet{floorCoveringGraphicSets.emplace_back(
        GraphicSet{graphicSetJson.at("displayName").get<std::string>(),
                   graphicSetJson.at("stringID").get<std::string>()},
        graphicSetJson.at("numericID"),
        std::array<GraphicRef, FloorCoveringGraphicSet::VARIATION_COUNT>{
            nullSprite, nullSprite, nullSprite, nullSprite, nullSprite,
            nullSprite, nullSprite, nullSprite})};

    // Add the graphics.
    std::size_t index{0};
    for (auto& graphicIDJson : graphicSetJson.at("graphicIDs").items()) {
        GraphicID graphicID{graphicIDJson.value().get<GraphicID>()};
        if (graphicID) {
            graphicSet.graphics[index] = getGraphic(graphicID);
        }
        else {
            // Empty slot. Set it to the null sprite.
            graphicSet.graphics[index] = {sprites[0]};
        }
        index++;
    }
}

void GraphicDataBase::parseWallGraphicSet(const nlohmann::json& graphicSetJson)
{
    Uint16 numericID{graphicSetJson.at("numericID")};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};
    std::string stringID{graphicSetJson.at("stringID").get<std::string>()};

    // Add the graphics.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    GraphicRef westGraphic{getGraphic(graphicIDJson[0].get<GraphicID>())};
    GraphicRef northGraphic{getGraphic(graphicIDJson[1].get<GraphicID>())};
    GraphicRef northwestGraphic{getGraphic(graphicIDJson[2].get<GraphicID>())};
    GraphicRef northeastGraphic{getGraphic(graphicIDJson[3].get<GraphicID>())};

    // Save the graphic set in the appropriate vector.
    wallGraphicSets.emplace_back(
        GraphicSet{displayName, stringID}, numericID,
        std::array<GraphicRef, Wall::Type::Count>{
            westGraphic, northGraphic, northwestGraphic, northeastGraphic});
}

void GraphicDataBase::parseObjectGraphicSet(
    const nlohmann::json& graphicSetJson)
{
    // Add a graphic set to the appropriate vector.
    GraphicRef nullSprite{sprites[0]};
    ObjectGraphicSet& graphicSet{objectGraphicSets.emplace_back(
        GraphicSet{graphicSetJson.at("displayName").get<std::string>(),
                   graphicSetJson.at("stringID").get<std::string>()},
        graphicSetJson.at("numericID"),
        std::array<GraphicRef, ObjectGraphicSet::VARIATION_COUNT>{
            nullSprite, nullSprite, nullSprite, nullSprite, nullSprite,
            nullSprite, nullSprite, nullSprite})};

    // Add the graphics.
    std::size_t index{0};
    for (auto& graphicIDJson : graphicSetJson.at("graphicIDs").items()) {
        GraphicID graphicID{graphicIDJson.value().get<GraphicID>()};
        if (graphicID) {
            graphicSet.graphics[index] = getGraphic(graphicID);
        }
        else {
            // Empty slot. Set it to the null sprite.
            graphicSet.graphics[index] = {sprites[0]};
        }
        index++;
    }
}

} // End namespace AM
