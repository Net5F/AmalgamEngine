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
, displayNames{}
, stringIDs{}
, stringMap{}
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
    // Note: This also sets emptySpriteIndex.
    parseJson(json);

    // Add the empty sprite.
    sprites[emptySpriteIndex].numericID = EMPTY_SPRITE_ID;
    displayNames[emptySpriteIndex] = "Empty";
    stringIDs[emptySpriteIndex] = "empty";
    stringMap.emplace(stringIDs[emptySpriteIndex], &sprites[emptySpriteIndex]);
}

const Sprite& SpriteDataBase::get(const std::string& stringID) const
{
    // Attempt to find the given string ID.
    auto it = stringMap.find(stringID);
    if (it == stringMap.end()) {
        LOG_FATAL("Failed to find sprite from string ID: %s", stringID.c_str());
    }

    return *(it->second);
}

const Sprite& SpriteDataBase::get(int numericID) const
{
    if (numericID == EMPTY_SPRITE_ID) {
        return sprites[emptySpriteIndex];
    }
    else if (numericID < 0 || numericID >= static_cast<int>(sprites.size())) {
        LOG_FATAL("Invalid numeric ID while getting sprite: %d", numericID);
    }

    return sprites[numericID];
}

const std::vector<Sprite>& SpriteDataBase::getAllSprites() const
{
    return sprites;
}

const std::string& SpriteDataBase::getDisplayName(int numericID) const
{
    if (numericID == EMPTY_SPRITE_ID) {
        return displayNames[emptySpriteIndex];
    }
    else if (numericID < 0
             || numericID >= static_cast<int>(displayNames.size())) {
        LOG_FATAL("Invalid numeric ID while getting sprite display name: %d",
                  numericID);
    }

    return displayNames[numericID];
}

const std::string& SpriteDataBase::getStringID(int numericID) const
{
    if (numericID == EMPTY_SPRITE_ID) {
        return stringIDs[emptySpriteIndex];
    }
    else if (numericID < 0 || numericID >= static_cast<int>(stringIDs.size())) {
        LOG_FATAL("Invalid numeric ID while getting sprite string ID: %d",
                  numericID);
    }

    return stringIDs[numericID];
}

void SpriteDataBase::parseJson(nlohmann::json& json)
{
    // Parse the json and catch any parsing errors.
    try {
        // Find the number of sprites in the json and resize our vectors.
        unsigned int spriteCount{getSpriteCount(json)};
        sprites.resize(spriteCount);
        displayNames.resize(spriteCount);
        stringIDs.resize(spriteCount);

        // Set the empty sprite's index to the end of the vector.
        emptySpriteIndex = (spriteCount - 1);

        // For every sprite sheet in the json.
        for (auto& sheetJson : json.at("spriteSheets").items()) {
            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value().at("sprites").items()) {
                // Parse the sprite's data and add it to our containers.
                parseSprite(spriteJson.value());
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL("Failure to parse SpriteDataBase.json: %s", e.what());
    }
}

void SpriteDataBase::parseSprite(const nlohmann::json& spriteJson)
{
    // Add the numeric identifier.
    Sprite sprite{};
    sprite.numericID = spriteJson.at("numericID");

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
    sprites[sprite.numericID] = sprite;

    // Save the display name and string ID in their vectors.
    displayNames[sprite.numericID]
        = spriteJson["displayName"].get<std::string>();
    stringIDs[sprite.numericID] = spriteJson.at("stringID").get<std::string>();

    // Add the sprite to the string ID map.
    stringMap.emplace(stringIDs[sprite.numericID], &sprites[sprite.numericID]);
}

unsigned int SpriteDataBase::getSpriteCount(const nlohmann::json& json)
{
    // Find the last sprite's numeric ID.
    const nlohmann::json& lastSpriteSheet{json.at("spriteSheets").back()};
    const nlohmann::json& lastSprite{lastSpriteSheet.at("sprites").back()};
    int lastNumericID{lastSprite.at("numericID")};

    // Count = lastNumericID + 1 (indexing) + 1 (empty sprite).
    return (lastNumericID + 1 + 1);
}

} // End namespace AM
