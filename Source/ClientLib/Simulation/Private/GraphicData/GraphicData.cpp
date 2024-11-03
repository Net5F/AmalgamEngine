#include "GraphicData.h"
#include "AssetCache.h"
#include "SpriteID.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"

namespace AM
{
namespace Client
{
GraphicData::GraphicData(const nlohmann::json& resourceDataJson,
                       AssetCache& assetCache)
: GraphicDataBase(resourceDataJson)
{
    // Parse the json structure to construct our sprite render data.
    parseJson(resourceDataJson, assetCache);
}

const SpriteRenderData& GraphicData::getRenderData(SpriteID numericID) const
{
    if (numericID >= renderData.size()) {
        LOG_ERROR("Invalid numeric ID while getting sprite render data: %d",
                  numericID);
        return renderData[0];
    }

    return renderData[numericID];
}

void GraphicData::parseJson(const nlohmann::json& json, AssetCache& assetCache)
{
    // Add the null sprite.
    renderData.emplace_back();

    // Parse the json and catch any parsing errors.
    try {
        // Resize our vector.
        renderData.resize(sprites.size());

        // Parse every sprite sheet in the json.
        std::string texturePath{};
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // Get this sheet's texture.
            texturePath = Paths::TEXTURE_DIR;
            texturePath += "SpriteSheets/";
            texturePath += sheetJson.value()["displayName"].get<std::string>();
            texturePath += ".png";

            TextureHandle texture{assetCache.requestTexture(texturePath)};
            if (!texture) {
                // Note: requestTexture will LOG_ERROR if the file isn't found.
                continue;
            }

            // Parse every sprite in this sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                parseSprite(spriteJson.value(), texturePath, texture);
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL(
            "Failed to parse sprites and sprite sets in ResourceData.json: %s",
            e.what());
    }
}

void GraphicData::parseSprite(const nlohmann::json& spriteJson,
                              const std::string& spriteSheetRelPath,
                              const TextureHandle& texture)
{
    // Get the numeric identifier.
    SpriteID numericID{spriteJson.at("numericID")};

    // Add the parent sprite sheet's path and texture.
    SpriteRenderData& spriteRenderData{renderData[numericID]};
    spriteRenderData.spriteSheetRelPath = spriteSheetRelPath;
    spriteRenderData.texture = texture;

    // Add this sprite's extent within the sprite sheet.
    spriteRenderData.textureExtent.x = spriteJson.at("textureExtent").at("x");
    spriteRenderData.textureExtent.y = spriteJson.at("textureExtent").at("y");
    spriteRenderData.textureExtent.w = spriteJson.at("textureExtent").at("w");
    spriteRenderData.textureExtent.h = spriteJson.at("textureExtent").at("h");

    // Add the stage origin.
    spriteRenderData.stageOrigin.x = spriteJson.at("stageX");
    spriteRenderData.stageOrigin.y = spriteJson.at("stageY");
}

} // End namespace Client
} // End namespace AM
