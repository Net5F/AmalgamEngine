#include "SpriteData.h"
#include "AssetCache.h"
#include "EmptySpriteID.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <fstream>

namespace AM
{
namespace Client
{
SpriteData::SpriteData(AssetCache& assetCache)
: SpriteDataBase()
{
    // Note: SpriteDataBase has already parsed the file and its data structures,
    //       but it didn't parse any rendering-related data.

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

    // Parse the json structure to construct our sprite render data.
    parseJson(json, assetCache);

    // Add the empty sprite.
    renderData[emptySpriteIndex] = SpriteRenderData{};
}

const SpriteRenderData& SpriteData::getRenderData(int numericID) const
{
    if (numericID == EMPTY_SPRITE_ID) {
        return renderData[emptySpriteIndex];
    }
    else if (numericID < 0
             || numericID >= static_cast<int>(renderData.size())) {
        LOG_FATAL("Invalid numeric ID while getting sprite render data: %d",
                  numericID);
    }

    return renderData[numericID];
}

void SpriteData::parseJson(nlohmann::json& json, AssetCache& assetCache)
{
    // Parse the json and catch any parsing errors.
    try {
        // Resize our vector.
        renderData.resize(emptySpriteIndex + 1);

        // For every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // Get this sheet's texture.
            std::string texturePath{Paths::TEXTURE_DIR};
            texturePath += sheetJson.value()["relPath"].get<std::string>();
            TextureHandle texture{assetCache.loadTexture(texturePath)};

            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                parseSprite(spriteJson.value(), texturePath, texture);
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL("Failure to parse SpriteData.json: %s", e.what());
    }
}

void SpriteData::parseSprite(const nlohmann::json& spriteJson,
                             const std::string& spriteSheetRelPath,
                             const TextureHandle& texture)
{
    // Get the numeric identifier.
    int numericID{spriteJson.at("numericID")};

    // Add the parent sprite sheet's path and texture.
    SpriteRenderData& spriteRenderData{renderData[numericID]};
    spriteRenderData.spriteSheetRelPath = spriteSheetRelPath;
    spriteRenderData.texture = texture;

    // Add this sprite's extent within the sprite sheet.
    spriteRenderData.textureExtent.x = spriteJson.at("textureExtent").at("x");
    spriteRenderData.textureExtent.y = spriteJson.at("textureExtent").at("y");
    spriteRenderData.textureExtent.w = spriteJson.at("textureExtent").at("w");
    spriteRenderData.textureExtent.h = spriteJson.at("textureExtent").at("h");

    // Add the Y offset.
    spriteRenderData.yOffset = spriteJson.at("yOffset");
}

} // End namespace Client
} // End namespace AM
