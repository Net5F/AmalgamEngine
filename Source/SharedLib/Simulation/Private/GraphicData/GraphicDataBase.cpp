#include "GraphicDataBase.h"
#include "StringTools.h"
#include "Paths.h"
#include "Log.h"
#include "AMAssert.h"
#include "nlohmann/json.hpp"

namespace detail
{
template<size_t, class T>
constexpr T&& identity(T&& x)
{
    return std::forward<T>(x);
}

template<class T, size_t... Indices>
constexpr auto array_repeat_impl(T&& x, std::index_sequence<Indices...>)
{
    return std::array{identity<Indices>(x)...};
}

} // namespace detail

// Used to fill our non-default-constructible GraphicRef arrays.
// Ref: https://stackoverflow.com/a/63821008/4258629
template<size_t N, class T>
constexpr auto constructAndFillArray(T&& x)
{
    return detail::array_repeat_impl(std::forward<T>(x),
                                     std::make_index_sequence<N>());
}

namespace AM
{
GraphicDataBase::GraphicDataBase(const nlohmann::json& resourceDataJson)
: sprites{}
, animations{}
, terrainGraphicSets{}
, floorGraphicSets{}
, wallGraphicSets{}
, objectGraphicSets{}
, spriteStringMap{}
, terrainGraphicSetStringMap{}
, floorGraphicSetStringMap{}
, wallGraphicSetStringMap{}
, objectGraphicSetStringMap{}
, workStringID{}
{
    // Parse the json structure to construct our sprites and animations.
    parseJson(resourceDataJson);
}

const Sprite& GraphicDataBase::getSprite(const std::string& stringID)
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    // Attempt to find the string ID.
    auto it{spriteStringMap.find(workStringID)};
    if (it == spriteStringMap.end()) {
        LOG_ERROR("Failed to find sprite with string ID: %s",
                  workStringID.c_str());
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

const Animation& GraphicDataBase::getAnimation(const std::string& stringID)
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    // Attempt to find the string ID.
    auto it{animationStringMap.find(workStringID)};
    if (it == animationStringMap.end()) {
        LOG_ERROR("Failed to find animation with string ID: %s",
                  workStringID.c_str());
        return animations[0];
    }

    return *(it->second);
}

const Animation& GraphicDataBase::getAnimation(AnimationID numericID) const
{
    if (numericID >= animations.size()) {
        LOG_ERROR("Invalid numeric ID while getting animation: %d", numericID);
        return animations[0];
    }

    return animations[numericID];
}

GraphicRef GraphicDataBase::getGraphic(GraphicID numericID) const
{
    if (isAnimationID(numericID)) {
        AnimationID animationID{toAnimationID(numericID)};
        return {getAnimation(animationID)};
    }
    else {
        SpriteID spriteID{toSpriteID(numericID)};
        return {getSprite(spriteID)};
    }
}

const TerrainGraphicSet&
    GraphicDataBase::getTerrainGraphicSet(const std::string& stringID)
{
    StringTools::deriveStringID(stringID, workStringID);
    auto it{terrainGraphicSetStringMap.find(workStringID)};
    if (it == terrainGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  workStringID.c_str());
        return terrainGraphicSets[0];
    }

    return *(it->second);
}

const FloorGraphicSet&
    GraphicDataBase::getFloorGraphicSet(const std::string& stringID)
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    auto it{floorGraphicSetStringMap.find(workStringID)};
    if (it == floorGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  workStringID.c_str());
        return floorGraphicSets[0];
    }

    return *(it->second);
}

const WallGraphicSet&
    GraphicDataBase::getWallGraphicSet(const std::string& stringID)
{
    StringTools::deriveStringID(stringID, workStringID);
    auto it{wallGraphicSetStringMap.find(workStringID)};
    if (it == wallGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  workStringID.c_str());
        return wallGraphicSets[0];
    }

    return *(it->second);
}

const ObjectGraphicSet&
    GraphicDataBase::getObjectGraphicSet(const std::string& stringID)
{
    StringTools::deriveStringID(stringID, workStringID);
    auto it{objectGraphicSetStringMap.find(workStringID)};
    if (it == objectGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  workStringID.c_str());
        return objectGraphicSets[0];
    }

    return *(it->second);
}

const EntityGraphicSet&
    GraphicDataBase::getEntityGraphicSet(const std::string& stringID)
{
    StringTools::deriveStringID(stringID, workStringID);
    auto it{entityGraphicSetStringMap.find(workStringID)};
    if (it == entityGraphicSetStringMap.end()) {
        LOG_ERROR("Failed to find graphic set with string ID: %s",
                  workStringID.c_str());
        return entityGraphicSets[0];
    }

    return *(it->second);
}

const TerrainGraphicSet& GraphicDataBase::getTerrainGraphicSet(
    TerrainGraphicSetID numericID) const
{
    if (numericID >= terrainGraphicSets.size()) {
        LOG_ERROR("Invalid numeric ID while getting graphic set: %d", numericID);
        return terrainGraphicSets[0];
    }

    return terrainGraphicSets[numericID];
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

const EntityGraphicSet&
    GraphicDataBase::getEntityGraphicSet(EntityGraphicSetID numericID) const
{
    if (numericID >= entityGraphicSets.size()) {
        LOG_ERROR("Invalid numeric ID while getting graphic set: %d", numericID);
        return entityGraphicSets[0];
    }

    return entityGraphicSets[numericID];
}

const std::vector<Sprite>& GraphicDataBase::getAllSprites() const
{
    return sprites;
}

const std::vector<TerrainGraphicSet>&
    GraphicDataBase::getAllTerrainGraphicSets() const
{
    return terrainGraphicSets;
}

const std::vector<FloorGraphicSet>& GraphicDataBase::getAllFloorGraphicSets() const
{
    return floorGraphicSets;
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

const std::vector<EntityGraphicSet>&
    GraphicDataBase::getAllEntityGraphicSets() const
{
    return entityGraphicSets;
}

void GraphicDataBase::parseJson(const nlohmann::json& json)
{
    // Add the null sprite, animation, and graphic sets.
    GraphicRef nullSprite{sprites.emplace_back("Null", "null", NULL_SPRITE_ID)};
    animations.emplace_back("Null", "null", NULL_ANIMATION_ID);
    terrainGraphicSets.emplace_back(
        GraphicSet{"Null", "null", NULL_TERRAIN_GRAPHIC_SET_ID},
        constructAndFillArray<Terrain::Height::Count>(nullSprite));
    floorGraphicSets.emplace_back(
        GraphicSet{"Null", "null", NULL_FLOOR_GRAPHIC_SET_ID},
        constructAndFillArray<FloorGraphicSet::VARIATION_COUNT>(nullSprite));
    wallGraphicSets.emplace_back(
        GraphicSet{"Null", "null", NULL_WALL_GRAPHIC_SET_ID},
        constructAndFillArray<Wall::Type::Count>(nullSprite));
    objectGraphicSets.emplace_back(
        GraphicSet{"Null", "null", NULL_OBJECT_GRAPHIC_SET_ID},
        constructAndFillArray<ObjectGraphicSet::VARIATION_COUNT>(nullSprite));
    entityGraphicSets.emplace_back(
        GraphicSet{"Null", "null", NULL_ENTITY_GRAPHIC_SET_ID});

    // Parse the json and catch any parsing errors.
    try {
        // Resize the vectors to fit everything in the json.
        resizeVectors(json);

        // Iterate every sprite sheet and add all of their sprites.
        for (auto& sheetJson : json.at("spriteSheets").items()) {
            for (auto& spriteJson : sheetJson.value().at("sprites").items()) {
                // Parse the sprite's data and add it to our containers.
                parseSprite(spriteJson.value());
            }
        }

        // Add every animation.
        for (auto& animationJson : json.at("animations").items()) {
            parseAnimation(animationJson.value());
        }

        // Add each type of graphic set.
        for (auto& terrainJson : json.at("terrain").items()) {
            parseTerrainGraphicSet(terrainJson.value());
        }
        for (auto& floorJson : json.at("floors").items()) {
            parseFloorGraphicSet(floorJson.value());
        }
        for (auto& wallJson : json.at("walls").items()) {
            parseWallGraphicSet(wallJson.value());
        }
        for (auto& objectJson : json.at("objects").items()) {
            parseObjectGraphicSet(objectJson.value());
        }
        for (auto& entityJson : json.at("entities").items()) {
            parseEntityGraphicSet(entityJson.value());
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
    for (const Animation& animation : animations) {
        animationStringMap.emplace(animation.stringID,
                                   &animations[animation.numericID]);
    }
    for (const TerrainGraphicSet& set : terrainGraphicSets) {
        terrainGraphicSetStringMap.emplace(
            set.stringID, &terrainGraphicSets[set.numericID]);
    }
    for (const FloorGraphicSet& set : floorGraphicSets) {
        floorGraphicSetStringMap.emplace(set.stringID,
                                        &floorGraphicSets[set.numericID]);
    }
    for (const WallGraphicSet& set : wallGraphicSets) {
        wallGraphicSetStringMap.emplace(set.stringID,
                                       &wallGraphicSets[set.numericID]);
    }
    for (const ObjectGraphicSet& set : objectGraphicSets) {
        objectGraphicSetStringMap.emplace(set.stringID,
                                         &objectGraphicSets[set.numericID]);
    }
    for (const EntityGraphicSet& set : entityGraphicSets) {
        entityGraphicSetStringMap.emplace(set.stringID,
                                          &entityGraphicSets[set.numericID]);
    }
}

void GraphicDataBase::resizeVectors(const nlohmann::json& json)
{
    // For each category of graphics, find the max ID and resize the vector.
    // Note: We have to find the max ID instead of just getting the array 
    //       size because there may be ID gaps.

    // Sprite
    GraphicRef nullSprite{sprites.at(NULL_SPRITE_ID)};
    SpriteID maxSpriteID{0};
    for (auto& sheetJson : json.at("spriteSheets").items()) {
        for (auto& spriteJson : sheetJson.value().at("sprites").items()) {
            SpriteID spriteID{spriteJson.value().at("numericID")};
            maxSpriteID = std::max(maxSpriteID, spriteID);
        }
    }
    sprites.resize(maxSpriteID + 1);

    // Animation
    AnimationID maxAnimationID{0};
    for (auto& animationJson : json.at("animations").items()) {
        AnimationID animationID{animationJson.value().at("numericID")};
        maxAnimationID = std::max(maxAnimationID, animationID);
    }
    animations.resize(maxAnimationID + 1);

    // Terrain
    // Note: We need to initialize them with constructAndFillArray() because
    //       GraphicRef has no default constructor (must always be valid).
    TerrainGraphicSetID maxTerrainID{0};
    for (auto& terrainJson : json.at("terrain").items()) {
        TerrainGraphicSetID terrainID{terrainJson.value().at("numericID")};
        maxTerrainID = std::max(maxTerrainID, terrainID);
    }
    TerrainGraphicSet terrainGraphicSet{.graphics{
        constructAndFillArray<Terrain::Height::Count>(nullSprite)}};
    terrainGraphicSets.resize(maxTerrainID + 1, terrainGraphicSet);

    // Floors
    FloorGraphicSetID maxFloorID{0};
    for (auto& floorJson : json.at("floors").items()) {
        FloorGraphicSetID floorID{floorJson.value().at("numericID")};
        maxFloorID = std::max(maxFloorID, floorID);
    }
    FloorGraphicSet floorGraphicSet{
        .graphics{constructAndFillArray<FloorGraphicSet::VARIATION_COUNT>(
            nullSprite)}};
    floorGraphicSets.resize(maxFloorID + 1, floorGraphicSet);

    // Walls
    WallGraphicSetID maxWallID{0};
    for (auto& wallJson : json.at("walls").items()) {
        WallGraphicSetID wallID{wallJson.value().at("numericID")};
        maxWallID = std::max(maxWallID, wallID);
    }
    WallGraphicSet wallGraphicSet{
        .graphics{constructAndFillArray<Wall::Type::Count>(nullSprite)}};
    wallGraphicSets.resize(maxWallID + 1, wallGraphicSet);

    // Objects
    ObjectGraphicSetID maxObjectID{0};
    for (auto& objectJson : json.at("objects").items()) {
        ObjectGraphicSetID objectID{objectJson.value().at("numericID")};
        maxObjectID = std::max(maxObjectID, objectID);
    }
    ObjectGraphicSet objectGraphicSet{
        .graphics{constructAndFillArray<ObjectGraphicSet::VARIATION_COUNT>(
            nullSprite)}};
    objectGraphicSets.resize(maxObjectID + 1, objectGraphicSet);

    // Entities
    EntityGraphicSetID maxEntityID{0};
    for (auto& entityJson : json.at("entities").items()) {
        EntityGraphicSetID entityID{entityJson.value().at("numericID")};
        maxEntityID = std::max(maxEntityID, entityID);
    }
    entityGraphicSets.resize(maxEntityID + 1);
}

void GraphicDataBase::parseSprite(const nlohmann::json& spriteJson)
{
    // Add the sprite to the sprites vector.
    SpriteID numericID{spriteJson.at("numericID")};
    Sprite& sprite{sprites.at(numericID)};

    // Add the display name and IDs.
    sprite.numericID = numericID;
    sprite.displayName = spriteJson.at("displayName").get<std::string>();
    StringTools::deriveStringID(sprite.displayName, sprite.stringID);

    // Add whether the sprite has a bounding box or not.
    sprite.collisionEnabled = spriteJson.at("collisionEnabled");

    // Add the model-space bounds.
    sprite.modelBounds.min.x = spriteJson.at("modelBounds").at("minX");
    sprite.modelBounds.max.x = spriteJson.at("modelBounds").at("maxX");
    sprite.modelBounds.min.y = spriteJson.at("modelBounds").at("minY");
    sprite.modelBounds.max.y = spriteJson.at("modelBounds").at("maxY");
    sprite.modelBounds.min.z = spriteJson.at("modelBounds").at("minZ");
    sprite.modelBounds.max.z = spriteJson.at("modelBounds").at("maxZ");
}

void GraphicDataBase::parseAnimation(const nlohmann::json& animationJson)
{
    // Add the animation to the animations vector.
    AnimationID numericID{animationJson.at("numericID")};
    Animation& animation{animations.at(numericID)};

    // Add the display name and IDs.
    animation.numericID = numericID;
    animation.displayName = animationJson.at("displayName").get<std::string>();
    StringTools::deriveStringID(animation.displayName, animation.stringID);

    // Add the frame count and fps.
    animation.frameCount = animationJson.at("frameCount");
    animation.fps = animationJson.at("fps");

    // Add the frames.
    // Note: If the animation is empty, the importer will give it a single 
    //       frame with the null sprite. This gets handled the same as any 
    //       other sprite by the renderer.
    const auto& frameNumbersJson{animationJson.at("frameNumbers")};
    const auto& spriteIdsJson{animationJson.at("spriteIDs")};
    for (std::size_t i{0}; i < frameNumbersJson.size(); ++i) {
        SpriteID spriteID{spriteIdsJson.at(i).get<SpriteID>()};
        Uint8 frameNumber{frameNumbersJson.at(i).get<Uint8>()};
        const Sprite& sprite{getSprite(spriteID)};
        animation.frames.emplace_back(frameNumber, sprite);
    }

    // Add whether the animation has a bounding box or not.
    animation.collisionEnabled = animationJson.at("collisionEnabled");

    // Add the model-space bounds.
    animation.modelBounds.min.x = animationJson.at("modelBounds").at("minX");
    animation.modelBounds.max.x = animationJson.at("modelBounds").at("maxX");
    animation.modelBounds.min.y = animationJson.at("modelBounds").at("minY");
    animation.modelBounds.max.y = animationJson.at("modelBounds").at("maxY");
    animation.modelBounds.min.z = animationJson.at("modelBounds").at("minZ");
    animation.modelBounds.max.z = animationJson.at("modelBounds").at("maxZ");
}

void GraphicDataBase::parseTerrainGraphicSet(const nlohmann::json& graphicSetJson)
{
    // Add a graphic set to the appropriate vector.
    TerrainGraphicSetID numericID{graphicSetJson.at("numericID")};
    TerrainGraphicSet& graphicSet{terrainGraphicSets.at(numericID)};
    graphicSet.numericID = numericID;
    graphicSet.displayName
        = graphicSetJson.at("displayName").get<std::string>();
    StringTools::deriveStringID(graphicSet.displayName, graphicSet.stringID);
    GraphicRef nullSprite{sprites[NULL_SPRITE_ID]};

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

void GraphicDataBase::parseFloorGraphicSet(
    const nlohmann::json& graphicSetJson)
{
    // Add a graphic set to the appropriate vector.
    FloorGraphicSetID numericID{graphicSetJson.at("numericID")};
    FloorGraphicSet& graphicSet{floorGraphicSets.at(numericID)};
    graphicSet.numericID = numericID;
    graphicSet.displayName
        = graphicSetJson.at("displayName").get<std::string>();
    StringTools::deriveStringID(graphicSet.displayName, graphicSet.stringID);
    GraphicRef nullSprite{sprites[NULL_SPRITE_ID]};

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
    // Collect the wall graphics.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    GraphicRef westGraphic{getGraphic(graphicIDJson[0].get<GraphicID>())};
    GraphicRef northGraphic{getGraphic(graphicIDJson[1].get<GraphicID>())};
    GraphicRef northwestGraphic{getGraphic(graphicIDJson[2].get<GraphicID>())};
    GraphicRef northeastGraphic{getGraphic(graphicIDJson[3].get<GraphicID>())};

    // Save the graphic set in the appropriate vector.
    WallGraphicSetID numericID{graphicSetJson.at("numericID")};
    WallGraphicSet& graphicSet{wallGraphicSets.at(numericID)};
    graphicSet.numericID = numericID;
    graphicSet.displayName
        = graphicSetJson.at("displayName").get<std::string>();
    StringTools::deriveStringID(graphicSet.displayName, graphicSet.stringID);
    graphicSet.graphics = std::array<GraphicRef, Wall::Type::Count>{
        westGraphic, northGraphic, northwestGraphic, northeastGraphic};
}

void GraphicDataBase::parseObjectGraphicSet(
    const nlohmann::json& graphicSetJson)
{
    // Add a graphic set to the appropriate vector.
    ObjectGraphicSetID numericID{graphicSetJson.at("numericID")};
    ObjectGraphicSet& graphicSet{objectGraphicSets.at(numericID)};
    graphicSet.numericID = numericID;
    graphicSet.displayName
        = graphicSetJson.at("displayName").get<std::string>();
    StringTools::deriveStringID(graphicSet.displayName, graphicSet.stringID);
    GraphicRef nullSprite{sprites[NULL_SPRITE_ID]};
    graphicSet.graphics
        = constructAndFillArray<ObjectGraphicSet::VARIATION_COUNT>(nullSprite);

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

void GraphicDataBase::parseEntityGraphicSet(
    const nlohmann::json& graphicSetJson)
{
    // Add a graphic set to the appropriate vector.
    EntityGraphicSetID numericID{graphicSetJson.at("numericID")};
    EntityGraphicSet& graphicSet{entityGraphicSets.at(numericID)};
    graphicSet.numericID = numericID;
    graphicSet.displayName
        = graphicSetJson.at("displayName").get<std::string>();
    StringTools::deriveStringID(graphicSet.displayName, graphicSet.stringID);

    // Add the graphics.
    const auto& graphicIDTypesJson{graphicSetJson.at("graphicIDTypes")};
    const auto& graphicIDValuesJson{graphicSetJson.at("graphicIDValues")};
    for (std::size_t i{0}; i < graphicIDTypesJson.size(); ++i) {
        EntityGraphicType graphicType{
            graphicIDTypesJson.at(i).get<EntityGraphicType>()};
        GraphicID graphicID{graphicIDValuesJson.at(i).get<GraphicID>()};

        graphicSet.graphics.emplace(graphicType, getGraphic(graphicID));
    }
}

} // End namespace AM
