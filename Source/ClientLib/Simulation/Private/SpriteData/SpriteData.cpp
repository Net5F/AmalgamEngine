#include "SpriteData.h"
#include "AssetCache.h"
#include "NullSpriteID.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"

namespace AM
{
namespace Client
{
SpriteData::SpriteData(const nlohmann::json& resourceDataJson,
                       AssetCache& assetCache)
: SpriteDataBase(resourceDataJson)
{
    // Parse the json structure to construct our sprite render data.
    parseJson(resourceDataJson, assetCache);
}

const SpriteRenderData& SpriteData::getRenderData(int numericID) const
{
    if (numericID == NULL_SPRITE_ID) {
        return renderData[nullSpriteIndex];
    }
    else if (numericID < 0
             || numericID >= static_cast<int>(renderData.size())) {
        LOG_FATAL("Invalid numeric ID while getting sprite render data: %d",
                  numericID);
    }

    return renderData[numericID];
}

void SpriteData::parseJson(const nlohmann::json& json, AssetCache& assetCache)
{
    // Parse the json and catch any parsing errors.
    try {
        // Resize our vector.
        renderData.resize(nullSpriteIndex + 1);

        // Parse every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // Get this sheet's texture.
            std::string texturePath{Paths::TEXTURE_DIR};
            texturePath += sheetJson.value()["relPath"].get<std::string>();
            TextureHandle texture{assetCache.loadTexture(texturePath)};

            // Parse every sprite in this sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                parseSprite(spriteJson.value(), texturePath, texture);
            }
        }

        // Add the null sprite.
        renderData[nullSpriteIndex] = SpriteRenderData{};
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL(
            "Failed to parse sprites and sprite sets in ResourceData.json: %s",
            e.what());
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
