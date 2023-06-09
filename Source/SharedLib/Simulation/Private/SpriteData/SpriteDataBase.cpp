#include "SpriteDataBase.h"
#include "EmptySpriteID.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <fstream>

namespace AM
{
SpriteDataBase::SpriteDataBase()
: emptySpriteIndex{0}
, sprites{}
, floorSpriteSets{}
, floorCoveringSpriteSets{}
, wallSpriteSets{}
, objectSpriteSets{}
, spriteStringMap{}
, floorSpriteSetStringMap{}
, floorCoveringSpriteSetStringMap{}
, wallSpriteSetStringMap{}
, objectSpriteSetStringMap{}
{
    // Open the file.
    std::string fullPath{Paths::BASE_PATH};
    fullPath += "SpriteData.json";
    std::ifstream workingFile(fullPath);
    if (!(workingFile.is_open())) {
        LOG_FATAL("Failed to open SpriteData.json");
    }

    // Parse the file into a json structure.
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(workingFile, nullptr, true);
    } catch (nlohmann::json::exception& e) {
        LOG_FATAL("Failed to parse SpriteData.json: %s", e.what());
    }

    // Parse the json structure to construct our sprites.
    parseJson(json);
}

const Sprite& SpriteDataBase::getSprite(const std::string& stringID) const
{
    // Attempt to find the given string ID.
    auto it{spriteStringMap.find(stringID)};
    if (it == spriteStringMap.end()) {
        LOG_FATAL("Failed to find sprite with string ID: %s", stringID.c_str());
    }

    return *(it->second);
}

const Sprite& SpriteDataBase::getSprite(int numericID) const
{
    if (numericID == EMPTY_SPRITE_ID) {
        return sprites[emptySpriteIndex];
    }
    else if (numericID < 0 || numericID >= static_cast<int>(sprites.size())) {
        LOG_FATAL("Invalid numeric ID while getting sprite: %d", numericID);
    }

    return sprites[numericID];
}

const FloorSpriteSet& SpriteDataBase::getFloorSpriteSet(const std::string& stringID) const
{
    auto it{floorSpriteSetStringMap.find(stringID)};
    if (it == floorSpriteSetStringMap.end()) {
        LOG_FATAL("Failed to find sprite set with string ID: %s", stringID.c_str());
    }

    return *(it->second);
}

const FloorCoveringSpriteSet& SpriteDataBase::getFloorCoveringSpriteSet(const std::string& stringID) const
{
    auto it{floorCoveringSpriteSetStringMap.find(stringID)};
    if (it == floorCoveringSpriteSetStringMap.end()) {
        LOG_FATAL("Failed to find sprite set with string ID: %s", stringID.c_str());
    }

    return *(it->second);
}

const WallSpriteSet& SpriteDataBase::getWallSpriteSet(const std::string& stringID) const
{
    auto it{wallSpriteSetStringMap.find(stringID)};
    if (it == wallSpriteSetStringMap.end()) {
        LOG_FATAL("Failed to find sprite set with string ID: %s", stringID.c_str());
    }

    return *(it->second);
}

const ObjectSpriteSet& SpriteDataBase::getObjectSpriteSet(const std::string& stringID) const
{
    auto it{objectSpriteSetStringMap.find(stringID)};
    if (it == objectSpriteSetStringMap.end()) {
        LOG_FATAL("Failed to find sprite set with string ID: %s", stringID.c_str());
    }

    return *(it->second);
}

const FloorSpriteSet& SpriteDataBase::getFloorSpriteSet(Uint16 numericID) const
{
    if (numericID >= floorSpriteSets.size()) {
        LOG_FATAL("Invalid numeric ID while getting sprite set: %d", numericID);
    }

    return floorSpriteSets[numericID];
}

const FloorCoveringSpriteSet&
    SpriteDataBase::getFloorCoveringSpriteSet(Uint16 numericID) const
{
    if (numericID >= floorCoveringSpriteSets.size()) {
        LOG_FATAL("Invalid numeric ID while getting sprite set: %d", numericID);
    }

    return floorCoveringSpriteSets[numericID];
}

const WallSpriteSet& SpriteDataBase::getWallSpriteSet(Uint16 numericID) const
{
    if (numericID >= wallSpriteSets.size()) {
        LOG_FATAL("Invalid numeric ID while getting sprite set: %d", numericID);
    }

    return wallSpriteSets[numericID];
}

const ObjectSpriteSet& SpriteDataBase::getObjectSpriteSet(Uint16 numericID) const
{
    if (numericID >= objectSpriteSets.size()) {
        LOG_FATAL("Invalid numeric ID while getting sprite set: %d", numericID);
    }

    return objectSpriteSets[numericID];
}

const std::vector<Sprite>& SpriteDataBase::getAllSprites() const
{
    return sprites;
}

const std::vector<FloorSpriteSet>& SpriteDataBase::getAllFloorSpriteSets() const
{
    return floorSpriteSets;
}

const std::vector<FloorCoveringSpriteSet>& SpriteDataBase::getAllFloorCoveringSpriteSets() const
{
    return floorCoveringSpriteSets;
}

const std::vector<WallSpriteSet>& SpriteDataBase::getAllWallSpriteSets() const
{
    return wallSpriteSets;
}

const std::vector<ObjectSpriteSet>& SpriteDataBase::getAllObjectSpriteSets() const
{
    return objectSpriteSets;
}

void SpriteDataBase::parseJson(nlohmann::json& json)
{
    // Parse the json and catch any parsing errors.
    try {
        // Iterate every sprite sheet and add all their sprites.
        for (auto& sheetJson : json.at("spriteSheets").items()) {
            for (auto& spriteJson : sheetJson.value().at("sprites").items()) {
                // Parse the sprite's data and add it to our containers.
                parseSprite(spriteJson.value());
            }
        }

        // Add each type of sprite set.
        for (auto& floorJson : json.at("floors").items()) {
            parseFloorSpriteSet(floorJson.value());
        }
        for (auto& floorCoveringJson : json.at("floorCoverings").items()) {
            parseFloorCoveringSpriteSet(floorCoveringJson.value());
        }
        for (auto& wallJson : json.at("walls").items()) {
            parseWallSpriteSet(wallJson.value());
        }
        for (auto& objectJson : json.at("objects").items()) {
            parseObjectSpriteSet(objectJson.value());
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL("Failure to parse SpriteDataBase.json: %s", e.what());
    }

    // Add the empty sprite and set the empty sprite index.
    sprites.push_back({"Empty", "empty", EMPTY_SPRITE_ID});
    emptySpriteIndex = static_cast<int>(sprites.size() - 1);

    // Add everything to the associated maps.
    for (const Sprite& sprite : sprites) {
        int numericID{sprite.numericID};
        if (numericID == EMPTY_SPRITE_ID) {
            numericID = emptySpriteIndex;
        }
        spriteStringMap.emplace(sprite.stringID, &sprites[numericID]);
    }
    for (const FloorSpriteSet& set : floorSpriteSets) {
        floorSpriteSetStringMap.emplace(set.stringID,
                                        &floorSpriteSets[set.numericID]);
    }
    for (const FloorCoveringSpriteSet& set : floorCoveringSpriteSets) {
        floorCoveringSpriteSetStringMap.emplace(
            set.stringID, &floorCoveringSpriteSets[set.numericID]);
    }
    for (const WallSpriteSet& set : wallSpriteSets) {
        wallSpriteSetStringMap.emplace(set.stringID,
                                       &wallSpriteSets[set.numericID]);
    }
    for (const ObjectSpriteSet& set : objectSpriteSets) {
        objectSpriteSetStringMap.emplace(set.stringID,
                                         &objectSpriteSets[set.numericID]);
    }
}

void SpriteDataBase::parseSprite(const nlohmann::json& spriteJson)
{
    // Add the display name and IDs.
    Sprite sprite{};
    sprite.numericID = spriteJson.at("numericID");
    sprite.displayName = spriteJson["displayName"].get<std::string>();
    sprite.stringID = spriteJson.at("stringID").get<std::string>();

    // Add whether the sprite has a bounding box or not.
    sprite.hasBoundingBox = spriteJson.at("hasBoundingBox");

    // Add the model-space bounds.
    sprite.modelBounds.minX = spriteJson.at("modelBounds").at("minX");
    sprite.modelBounds.maxX = spriteJson.at("modelBounds").at("maxX");
    sprite.modelBounds.minY = spriteJson.at("modelBounds").at("minY");
    sprite.modelBounds.maxY = spriteJson.at("modelBounds").at("maxY");
    sprite.modelBounds.minZ = spriteJson.at("modelBounds").at("minZ");
    sprite.modelBounds.maxZ = spriteJson.at("modelBounds").at("maxZ");

    // Save the sprite in the sprites vector.
    sprites.push_back(sprite);
}

void SpriteDataBase::parseFloorSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{spriteSetJson.at("numericID")};
    std::string displayName{spriteSetJson["displayName"].get<std::string>()};
    std::string stringID{spriteSetJson.at("stringID").get<std::string>()};

    // Add the sprite set's sprites.
    // Note: Floors just have 1 sprite, but the json uses an array in case we
    //       want to add variations in the future.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    const Sprite& sprite{getSprite(spriteIDJson[0].get<int>())};

    // Save the sprite set in the appropriate vector.
    floorSpriteSets.emplace_back(SpriteSet{displayName, stringID, numericID},
                                 sprite);
}

void SpriteDataBase::parseFloorCoveringSpriteSet(const nlohmann::json& spriteSetJson)
{
    // Add a sprite set to the appropriate vector.
    FloorCoveringSpriteSet& spriteSet{floorCoveringSpriteSets.emplace_back()};
    spriteSet.numericID = spriteSetJson.at("numericID");
    spriteSet.displayName = spriteSetJson["displayName"].get<std::string>();
    spriteSet.stringID = spriteSetJson.at("stringID").get<std::string>();

    // Add the sprite set's sprites.
    std::size_t index{0};
    for (auto& spriteIDJson : spriteSetJson.at("spriteIDs").items()) {
        int spriteID{spriteIDJson.value().get<int>()};
        if (spriteID == EMPTY_SPRITE_ID) {
            spriteSet.sprites[index] = nullptr;
        }
        else {
            spriteSet.sprites[index] = &getSprite(spriteID);
        }
        index++;
    }
}

void SpriteDataBase::parseWallSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{spriteSetJson.at("numericID")};
    std::string displayName{spriteSetJson["displayName"].get<std::string>()};
    std::string stringID{spriteSetJson.at("stringID").get<std::string>()};

    // Add the sprite set's sprites.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    const Sprite& westSprite{getSprite(spriteIDJson[0].get<int>())};
    const Sprite& northSprite{getSprite(spriteIDJson[1].get<int>())};
    const Sprite& northwestSprite{getSprite(spriteIDJson[2].get<int>())};
    const Sprite& northeastSprite{getSprite(spriteIDJson[3].get<int>())};

    // Save the sprite set in the appropriate vector.
    WallSpriteSet& spriteSet{wallSpriteSets.emplace_back(
        SpriteSet{displayName, stringID, numericID},
        std::array<std::reference_wrapper<const Sprite>, Wall::Type::Count>{
            westSprite, northSprite, northwestSprite, northeastSprite})};
}

void SpriteDataBase::parseObjectSpriteSet(const nlohmann::json& spriteSetJson)
{
    // Add a sprite set to the appropriate vector.
    ObjectSpriteSet& spriteSet{objectSpriteSets.emplace_back()};
    spriteSet.numericID = spriteSetJson.at("numericID");
    spriteSet.displayName = spriteSetJson["displayName"].get<std::string>();
    spriteSet.stringID = spriteSetJson.at("stringID").get<std::string>();

    // Add the sprite set's sprites.
    std::size_t index{0};
    for (auto& spriteIDJson : spriteSetJson.at("spriteIDs").items()) {
        int spriteID{spriteIDJson.value().get<int>()};
        if (spriteID == EMPTY_SPRITE_ID) {
            spriteSet.sprites[index] = nullptr;
        }
        else {
            spriteSet.sprites[index] = &getSprite(spriteID);
        }
        index++;
    }
}

} // End namespace AM
