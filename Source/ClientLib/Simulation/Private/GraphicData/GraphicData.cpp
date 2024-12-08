#include "GraphicData.h"
#include "AssetCache.h"
#include "SpriteID.h"
#include "Config.h"
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

const SpriteRenderData&
    GraphicData::getSpriteRenderData(SpriteID numericID) const
{
    if (numericID >= spriteRenderData.size()) {
        LOG_ERROR("Invalid numeric ID while getting sprite render data: %d",
                  numericID);
        return spriteRenderData[0];
    }

    return spriteRenderData[numericID];
}

const AnimationRenderData&
    GraphicData::getAnimationRenderData(AnimationID numericID) const
{
    if (numericID >= animationRenderData.size()) {
        LOG_ERROR("Invalid numeric ID while getting animation render data: %d",
                  numericID);
        return animationRenderData[0];
    }

    return animationRenderData[numericID];
}

void GraphicData::parseJson(const nlohmann::json& json, AssetCache& assetCache)
{
    // Add the null sprite.
    spriteRenderData.emplace_back();

    // Parse the json and catch any parsing errors.
    try {
        // Resize our vectors.
        spriteRenderData.resize(sprites.size());
        animationRenderData.resize(animations.size());

        // Parse every sprite sheet in the json.
        std::string texturePath{};
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // Get this sheet's texture.
            texturePath = Paths::TEXTURE_DIR;
            texturePath += "SpriteSheets/";
            texturePath += sheetJson.value()["displayName"].get<std::string>();
            texturePath += ".png";

            TextureHandle texture{};
            texture = assetCache.requestTexture(
                texturePath, Config::SPRITE_SCALING_QUALITY);
            if (!texture) {
                // Note: requestTexture will LOG_ERROR if the file isn't found.
                continue;
            }

            // Parse every sprite in this sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                parseSprite(spriteJson.value(), texturePath, texture);
            }
        }
        
        // Parse every animation in the json.
        for (auto& animationJson : json["animations"].items()) {
            parseAnimation(animationJson.value());
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
    SpriteRenderData& renderData{spriteRenderData[numericID]};
    renderData.spriteSheetRelPath = spriteSheetRelPath;
    renderData.texture = texture;

    // Add this sprite's extent within the sprite sheet.
    renderData.textureExtent.x = spriteJson.at("textureExtent").at("x");
    renderData.textureExtent.y = spriteJson.at("textureExtent").at("y");
    renderData.textureExtent.w = spriteJson.at("textureExtent").at("w");
    renderData.textureExtent.h = spriteJson.at("textureExtent").at("h");

    // Add the stage origin.
    renderData.stageOrigin.x = spriteJson.at("stageX");
    renderData.stageOrigin.y = spriteJson.at("stageY");

    // Add the premultiply alpha setting.
    renderData.premultiplyAlpha = spriteJson.at("premultiplyAlpha");
}

void GraphicData::parseAnimation(const nlohmann::json& animationJson)
{
    // Get the numeric identifier.
    AnimationID numericID{animationJson.at("numericID")};

    // Add the entity alignment anchor.
    AnimationRenderData& renderData{animationRenderData[numericID]};
    if (animationJson.contains("entityAlignmentAnchor")) {
        renderData.entityAlignmentAnchor
            = {animationJson.at("entityAlignmentAnchor").at("x"),
               animationJson.at("entityAlignmentAnchor").at("y"),
               animationJson.at("entityAlignmentAnchor").at("z")};
    }
}

Vector3
    GraphicData::getRenderAlignmentOffset(EntityGraphicSetID setID,
                                          EntityGraphicType graphicType) const
{
    const EntityGraphicSet& graphicSet{getEntityGraphicSet(setID)};
    if (!(graphicSet.graphics.contains(EntityGraphicType::IdleSouth))) {
        LOG_ERROR("Entity graphic set is missing IdleSouth: %s.",
                  graphicSet.displayName.c_str());
        return {};
    }
    if (!(graphicSet.graphics.contains(graphicType))) {
        LOG_ERROR("Entity graphic set is missing requested type: %s, %u.",
                  graphicSet.displayName.c_str(), graphicType);
        return {};
    }

    // If the requested graphic is a Sprite, return 0 (sprites don't have 
    // alignment anchors).
    const GraphicRef& graphicRef{graphicSet.graphics.at(graphicType)};
    if (std::holds_alternative<std::reference_wrapper<const Sprite>>(
            graphicRef)) {
        return {};
    }

    // If the requested Animation doesn't have an alignment anchor, return 0.
    const auto& animation{
        std::get<std::reference_wrapper<const Animation>>(graphicRef)};
    const AnimationRenderData& renderData{
        getAnimationRenderData(animation.get().numericID)};
    if (!(renderData.entityAlignmentAnchor)) {
        return {};
    }

    // Return the difference between the IdleSouth graphic's bottom center and 
    // the requested animation's alignment anchor.
    Vector3 idleSouthBottomCenter{
        graphicSet.graphics.at(EntityGraphicType::IdleSouth)
            .getModelBounds()
            .getBottomCenterPoint()};
    Vector3 alignmentAnchor{renderData.entityAlignmentAnchor.value()};
    return (idleSouthBottomCenter - alignmentAnchor);
}

} // End namespace Client
} // End namespace AM
