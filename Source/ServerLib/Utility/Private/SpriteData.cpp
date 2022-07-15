#include "SpriteData.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <fstream>

namespace AM
{
namespace Server
{
SpriteData::SpriteData()
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
    }
    catch (nlohmann::json::exception& e) {
        LOG_FATAL("Failed to parse SpriteData.json: %s", e.what());
    }

    // Parse the json structure to construct our sprites.
    parseJson(json);

    // Add the empty sprite.
    sprites.push_back(Sprite{});
    Sprite& emptySprite{sprites.back()};
    emptySprite.displayName = "Empty";
    emptySprite.stringID = "empty";
    emptySprite.numericID = EMPTY_SPRITE_ID;

    stringMap.emplace(emptySprite.stringID, &emptySprite);
    numericMap.emplace(emptySprite.numericID, &emptySprite);
}

const Sprite& SpriteData::get(const std::string& stringID)
{
    // Attempt to find the given string ID.
    auto it = stringMap.find(stringID);
    if (it == stringMap.end()) {
        LOG_FATAL("Failed to find sprite string ID: %s", stringID.c_str());
    }

    return *(it->second);
}

const Sprite& SpriteData::get(int numericID)
{
    // Attempt to find the given string ID.
    auto it = numericMap.find(numericID);
    if (it == numericMap.end()) {
        LOG_FATAL("Failed to find sprite numeric ID: %d", numericID);
    }

    return *(it->second);
}

void SpriteData::parseJson(nlohmann::json& json)
{
    // Parse the json and catch any parsing errors.
    try {
        // For every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                parseSprite(spriteJson.value());
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL("Parse failure - %s", e.what());
    }

    // Fill the maps with pointers to all the sprites.
    // Note: This must be done after filling the sprites vector, since a
    //       push_back() can cause a re-allocation which invalidates pointers.
    for (Sprite& sprite : sprites) {
        stringMap.emplace(sprite.stringID, &sprite);
        numericMap.emplace(sprite.numericID, &sprite);
    }
}

void SpriteData::parseSprite(const nlohmann::json& spriteJson)
{
    // Add the parent sprite sheet's texture.
    Sprite sprite{};

    // Add the display name.
    sprite.displayName = spriteJson["displayName"].get<std::string>();

    // Add the string identifier.
    sprite.stringID = spriteJson["stringID"].get<std::string>();

    // Add the numeric identifier.
    sprite.numericID = spriteJson["numericID"];

    // Add whether the sprite has a bounding box or not.
    sprite.hasBoundingBox = spriteJson["hasBoundingBox"];

    // Add the model-space bounds.
    sprite.modelBounds.minX = spriteJson["modelBounds"]["minX"];
    sprite.modelBounds.maxX = spriteJson["modelBounds"]["maxX"];
    sprite.modelBounds.minY = spriteJson["modelBounds"]["minY"];
    sprite.modelBounds.maxY = spriteJson["modelBounds"]["maxY"];
    sprite.modelBounds.minZ = spriteJson["modelBounds"]["minZ"];
    sprite.modelBounds.maxZ = spriteJson["modelBounds"]["maxZ"];

    // Save the sprite in the sprites vector.
    sprites.push_back(sprite);
}

} // End namespace Server
} // End namespace AM
