#include "SpriteData.h"
#include "AssetCache.h"
#include "Paths.h"
#include "Log.h"

#include <fstream>

#include "nlohmann/json.hpp"

namespace AM
{
namespace Client
{

SpriteData::SpriteData(AssetCache& assetCache)
{
    // Open the file.
    std::string fullPath{Paths::BASE_PATH};
    fullPath += "SpriteData.json";
    std::ifstream workingFile(fullPath);
    if (!(workingFile.is_open())) {
        LOG_ERROR("Failed to open SpriteData.json");
    }

    // Parse the file into a json structure.
    nlohmann::json json = nlohmann::json::parse(workingFile, nullptr, false);
    if (json.is_discarded()) {
        LOG_ERROR("SpriteData.json is not valid JSON.");
    }

    // Parse the json structure to construct our sprites.
    parseJson(json, assetCache);
}

const Sprite& SpriteData::get(const std::string& stringId)
{
    // Attempt to find the given string ID.
    auto it = stringMap.find(stringId);
    if (it == stringMap.end()) {
        LOG_ERROR("Failed to find sprite string ID: %s", stringId.c_str());
    }

    return *(it->second);
}

const Sprite& SpriteData::get(int numericId)
{
    // Attempt to find the given string ID.
    auto it = numericMap.find(numericId);
    if (it == numericMap.end()) {
        LOG_ERROR("Failed to find sprite numeric ID: %d", numericId);
    }

    return *(it->second);
}

void SpriteData::parseJson(nlohmann::json& json, AssetCache& assetCache)
{
    try {
        // For every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // Get this sheet's texture.
            std::string texturePath { Paths::TEXTURE_DIR };
            texturePath += sheetJson.value()["relPath"].get<std::string>();
            TextureHandle texture{assetCache.loadTexture(texturePath)};

            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                // Add the parent sprite sheet's texture.
                Sprite sprite{};
                sprite.texture = texture;

                // Add the display name.
                sprite.displayName
                    = spriteJson.value()["displayName"].get<std::string>();

                // Add the string identifier.
                sprite.stringId
                    = spriteJson.value()["stringId"].get<std::string>();

                // Add the numeric identifier.
                sprite.numericId = spriteJson.value()["numericId"];

                // Add this sprite's extent within the sprite sheet.
                sprite.textureExtent.x
                    = spriteJson.value()["textureExtent"]["x"];
                sprite.textureExtent.y
                    = spriteJson.value()["textureExtent"]["y"];
                sprite.textureExtent.w
                    = spriteJson.value()["textureExtent"]["w"];
                sprite.textureExtent.h
                    = spriteJson.value()["textureExtent"]["h"];

                // Add the Y offset.
                sprite.yOffset = spriteJson.value()["yOffset"];

                // Add the model-space bounds.
                sprite.modelBounds.minX
                    = spriteJson.value()["modelBounds"]["minX"];
                sprite.modelBounds.maxX
                    = spriteJson.value()["modelBounds"]["maxX"];
                sprite.modelBounds.minY
                    = spriteJson.value()["modelBounds"]["minY"];
                sprite.modelBounds.maxY
                    = spriteJson.value()["modelBounds"]["maxY"];
                sprite.modelBounds.minZ
                    = spriteJson.value()["modelBounds"]["minZ"];
                sprite.modelBounds.maxZ
                    = spriteJson.value()["modelBounds"]["maxZ"];

                // Save the sprite in the sprites vector.
                sprites.push_back(sprite);

                // Add pointers to the new sprite to the maps.
                const Sprite* spritePtr{&(sprites.back())};
                stringMap.emplace(spritePtr->stringId, spritePtr);
                numericMap.emplace(spritePtr->numericId, spritePtr);
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_ERROR("Parse failure - %s", e.what());
    }
}

} // End namespace Client
} // End namespace AM
