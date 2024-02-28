#include "SpriteModel.h"
#include "DataModel.h"
#include "GraphicSetModel.h"
#include "SpriteID.h"
#include "nlohmann/json.hpp"
#include <SDL_render.h>
#include <SDL_image.h>
#include <filesystem>
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{
SpriteModel::SpriteModel(DataModel& inDataModel,
                         GraphicSetModel& inGraphicSetModel,
                         SDL_Renderer* inSdlRenderer)
: graphicSetModel{inGraphicSetModel}
, dataModel{inDataModel}
, sdlRenderer{inSdlRenderer}
, spriteSheetMap{}
, spriteMap{}
, sheetIDPool{MAX_SPRITE_SHEETS}
, spriteIDPool{MAX_SPRITES}
, errorString{}
, sheetAddedSig{}
, sheetRemovedSig{}
, spriteRemovedSig{}
, spriteDisplayNameChangedSig{}
, spriteModelBoundsIDChangedSig{}
, spriteCustomModelBoundsChangedSig{}
, spriteCollisionEnabledChangedSig{}
, sheetAdded{sheetAddedSig}
, sheetRemoved{sheetRemovedSig}
, spriteRemoved{spriteRemovedSig}
, spriteDisplayNameChanged{spriteDisplayNameChangedSig}
, spriteModelBoundsIDChanged{spriteModelBoundsIDChangedSig}
, spriteCustomModelBoundsChanged{spriteCustomModelBoundsChangedSig}
, spriteCollisionEnabledChanged{spriteCollisionEnabledChangedSig}
{
    // Reserve the null sprite's ID (the engine provides it in code, so we don't
    // need it in the json).
    spriteIDPool.reserveID();
}

bool SpriteModel::load(const nlohmann::json& json)
{
    try {
        // Iterate every sprite sheet and add all their sprites.
        for (auto& sheetJson : json.at("spriteSheets").items()) {
            if (!parseSpriteSheet(sheetJson.value())) {
                return false;
            }
        }
    } catch (nlohmann::json::exception& e) {
        resetModelState();
        errorString = "Parse failure - ";
        errorString += e.what();
        return false;
    }

    return true;
}

void SpriteModel::save(nlohmann::json& json)
{
    json["spriteSheets"] = nlohmann::json::array();

    /* Fill the json with our current model data. */
    // For each sprite sheet.
    int i{0};
    SpriteID spriteID{1};
    for (auto& sheetPair : spriteSheetMap) {
        // Add this sheet's relative path.
        EditorSpriteSheet& spriteSheet{sheetPair.second};
        json["spriteSheets"][i]["relPath"] = spriteSheet.relPath;

        // For each sprite in this sheet.
        for (std::size_t j = 0; j < spriteSheet.spriteIDs.size(); ++j) {
            // Add the display name.
            EditorSprite& sprite{spriteMap[spriteSheet.spriteIDs[j]]};
            json["spriteSheets"][i]["sprites"][j]["displayName"]
                = sprite.displayName;

            // Derive the string ID from the display name and add it.
            json["spriteSheets"][i]["sprites"][j]["stringID"]
                = DataModel::deriveStringID(sprite.displayName);

            // Add the numeric ID.
            json["spriteSheets"][i]["sprites"][j]["numericID"]
                = spriteID++;

            // Add the sprite sheet texture extent.
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["x"]
                = sprite.textureExtent.x;
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["y"]
                = sprite.textureExtent.y;
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["w"]
                = sprite.textureExtent.w;
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["h"]
                = sprite.textureExtent.h;

            // Add the Y offset.
            json["spriteSheets"][i]["sprites"][j]["yOffset"] = sprite.yOffset;

            // Add collisionEnabled.
            json["spriteSheets"][i]["sprites"][j]["collisionEnabled"]
                = sprite.collisionEnabled;

            // Add modelBoundsID.
            // Note: The engine doesn't use this, but this editor needs it.
            json["spriteSheets"][i]["sprites"][j]["modelBoundsID"]
                = sprite.modelBoundsID;

            // Add the model-space bounds.
            // Note: This will either be a shared bounding box or a custom one 
            //       depending on modelBoundsID. The engine doesn't care either 
            //       way, it just needs to know what the bounds are. 
            const BoundingBox& spriteModelBounds{
                sprite.getModelBounds(dataModel.boundingBoxModel)}; 
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minX"]
                = spriteModelBounds.minX;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxX"]
                = spriteModelBounds.maxX;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minY"]
                = spriteModelBounds.minY;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxY"]
                = spriteModelBounds.maxY;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minZ"]
                = spriteModelBounds.minZ;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxZ"]
                = spriteModelBounds.maxZ;
        }

        i++;
    }
}

bool SpriteModel::addSpriteSheet(const std::string& relPath,
                                 const std::string& spriteWidth,
                                 const std::string& spriteHeight,
                                 const std::string& yOffset,
                                 const std::string& baseName)
{
    /* Validate the data. */
    // Check if we already have the given sheet.
    for (const auto& sheetPair : spriteSheetMap) {
        if (sheetPair.second.relPath == relPath) {
            errorString = "Error: Path conflicts with existing sprite sheet.";
            return false;
        }
    }

    // Prepend the texture directory to the given relative path.
    std::string fullPath{dataModel.getWorkingTexturesDir()};
    fullPath += relPath;

    // Validate that the file at the given path is a valid texture.
    int sheetWidth{0};
    int sheetHeight{0};
    SDL_Texture* sheetTexture{IMG_LoadTexture(sdlRenderer, fullPath.c_str())};
    if (sheetTexture != nullptr) {
        // Save the texture size for later.
        SDL_QueryTexture(sheetTexture, nullptr, nullptr, &sheetWidth,
                         &sheetHeight);

        // We don't need the actual texture right now, destroy it.
        SDL_DestroyTexture(sheetTexture);
    }
    else {
        errorString = "Error: File at given path is not a valid image. Path: ";
        errorString += dataModel.getWorkingTexturesDir();
        errorString += relPath;
        return false;
    }

    // Validate the width/height/yOffset.
    int spriteWidthI{0};
    int spriteHeightI{0};
    int yOffsetI{0};
    try {
        spriteWidthI = std::stoi(spriteWidth);
        spriteHeightI = std::stoi(spriteHeight);
        yOffsetI = std::stoi(yOffset);
    } catch (std::exception&) {
        errorString
            = "Error: Width, height, or Y offset is not a valid integer.";
        return false;
    }

    // Validate the size of the texture.
    if ((spriteWidthI > sheetWidth) || (spriteHeightI > sheetHeight)) {
        errorString = "Error: Sheet must be larger than sprite size.";
        return false;
    }

    /* Add the sprite sheet and sprites. */
    int sheetID{static_cast<int>(sheetIDPool.reserveID())};
    spriteSheetMap.emplace(sheetID, EditorSpriteSheet{relPath, {}});

    // For each sprite in this texture.
    EditorSpriteSheet& spriteSheet{spriteSheetMap[sheetID]};
    int spriteCount{0};
    for (int y = 0; y <= (sheetHeight - spriteHeightI); y += spriteHeightI) {
        for (int x = 0; x <= (sheetWidth - spriteWidthI); x += spriteWidthI) {
            // Build the sprite's display name (baseName_count).
            std::string displayName{baseName};
            displayName += std::to_string(spriteCount);

            // Find the sprite's extent within the sheet texture.
            SDL_Rect textureExtent{x, y, spriteWidthI, spriteHeightI};

            // Default to a non-0 bounding box so it's easier to click.
            static constexpr BoundingBox defaultBox{0, 20, 0, 20, 0, 20};

            // Add the sprite to the map and sheet.
            SpriteID spriteID{static_cast<SpriteID>(spriteIDPool.reserveID())};
            spriteMap.emplace(
                spriteID, EditorSprite{spriteID, spriteSheet.relPath,
                                       displayName, textureExtent, yOffsetI,
                                       true, NULL_BOUNDING_BOX_ID, defaultBox});
            spriteSheet.spriteIDs.push_back(spriteID);

            // Increment the count (used for the display name).
            spriteCount++;
        }
    }

    // Signal the new sheet to the UI.
    sheetAddedSig.publish(sheetID, spriteSheet);

    return true;
}

void SpriteModel::remSpriteSheet(int sheetID)
{
    // Find the sheet in the map.
    auto sheetIt{spriteSheetMap.find(sheetID)};
    if (sheetIt == spriteSheetMap.end()) {
        LOG_FATAL("Invalid ID while removing sprite sheet.");
    }

    // Erase all of the sheet's sprites.
    for (SpriteID spriteID : sheetIt->second.spriteIDs) {
        remSprite(spriteID);
    }

    // Erase the sheet.
    spriteSheetMap.erase(sheetIt);

    // Signal that the sheet was erased.
    sheetRemovedSig.publish(sheetID);
}

void SpriteModel::remSprite(SpriteID spriteID)
{
    // Find the sprite in the map.
    auto spriteIt{spriteMap.find(spriteID)};
    if (spriteIt == spriteMap.end()) {
        LOG_FATAL("Invalid ID while removing sprite.");
    }

    // Erase the sprite.
    spriteMap.erase(spriteIt);

    // Signal that the sprite was erased.
    spriteRemovedSig.publish(spriteID);

    // Clear this sprite from any sets that reference it.
    graphicSetModel.removeGraphicIDFromSets(toGraphicID(spriteID));
}

const EditorSprite& SpriteModel::getSprite(SpriteID spriteID)
{
    auto spriteIt{spriteMap.find(spriteID)};
    if (spriteIt == spriteMap.end()) {
        LOG_FATAL("Tried to get sprite with invalid ID: %d", spriteID);
    }

    return spriteIt->second;
}

void SpriteModel::setSpriteDisplayName(SpriteID spriteID,
                                       const std::string& newDisplayName)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set name using invalid sprite ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!spriteNameIsUnique(spriteID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    EditorSprite& sprite{spritePair->second};
    sprite.displayName = uniqueDisplayName;

    // Signal the change.
    spriteDisplayNameChangedSig.publish(spriteID, sprite.displayName);
}

void SpriteModel::setSpriteModelBoundsID(SpriteID spriteID,
                                         BoundingBoxID newModelBoundsID)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set modelBoundsID using invalid sprite ID.");
    }

    // Set the new ID and signal the change.
    EditorSprite& sprite{spritePair->second};
    sprite.modelBoundsID = newModelBoundsID;

    spriteModelBoundsIDChangedSig.publish(spriteID, newModelBoundsID);
}

void SpriteModel::setSpriteCustomModelBounds(SpriteID spriteID,
                                       const BoundingBox& newModelBounds)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set customModelBounds using invalid sprite ID.");
    }

    // Set the new model bounds and signal the change.
    EditorSprite& sprite{spritePair->second};
    sprite.customModelBounds = newModelBounds;

    spriteCustomModelBoundsChangedSig.publish(spriteID, newModelBounds);
}

void SpriteModel::setSpriteCollisionEnabled(SpriteID spriteID,
                                            bool newCollisionEnabled)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set collisionEnabled using invalid sprite ID.");
    }

    // Set the new collisionEnabled and signal the change.
    EditorSprite& sprite{spritePair->second};
    sprite.collisionEnabled = newCollisionEnabled;

    spriteCollisionEnabledChangedSig.publish(spriteID, newCollisionEnabled);
}

void SpriteModel::resetModelState()
{
    spriteSheetMap.clear();
    spriteMap.clear();
    sheetIDPool.freeAllIDs();
    spriteIDPool.freeAllIDs();
}

const std::string& SpriteModel::getErrorString()
{
    return errorString;
}

bool SpriteModel::parseSpriteSheet(const nlohmann::json& sheetJson)
{
    int sheetID{static_cast<int>(sheetIDPool.reserveID())};
    spriteSheetMap.emplace(sheetID, EditorSpriteSheet{});
    EditorSpriteSheet& spriteSheet{spriteSheetMap[sheetID]};

    // Add this sheet's relative path.
    spriteSheet.relPath = sheetJson.at("relPath").get<std::string>();
    if (!(dataModel.validateRelPath(spriteSheet.relPath))) {
        return false;
    }

    // For every sprite in the sheet.
    for (auto& spriteJson : sheetJson.at("sprites").items()) {
        if (!parseSprite(spriteJson.value(), spriteSheet)) {
            return false;
        }
    }

    // Signal the new sheet to the UI.
    sheetAddedSig.publish(sheetID, spriteSheet);

    return true;
}

bool SpriteModel::parseSprite(const nlohmann::json& spriteJson,
                              EditorSpriteSheet& spriteSheet)
{
    // Mark the sprite's ID as reserved so it doesn't get reused.
    SpriteID spriteID{spriteJson.at("numericID").get<SpriteID>()};
    spriteIDPool.markIDAsReserved(spriteID);

    spriteMap.emplace(spriteID, EditorSprite{spriteID, spriteSheet.relPath});
    spriteSheet.spriteIDs.push_back(spriteID);
    EditorSprite& sprite{spriteMap[spriteID]};

    // If the display name isn't unique, fail.
    std::string displayName{spriteJson.at("displayName").get<std::string>()};
    if (!spriteNameIsUnique(spriteID, displayName)) {
        errorString = "Sprite display name isn't unique: ";
        errorString += sprite.displayName.c_str();
        return false;
    }

    // Add the display name.
    sprite.displayName = displayName;

    // Add this sprite's extent within the sprite sheet.
    sprite.textureExtent.x = spriteJson.at("textureExtent").at("x");
    sprite.textureExtent.y = spriteJson.at("textureExtent").at("y");
    sprite.textureExtent.w = spriteJson.at("textureExtent").at("w");
    sprite.textureExtent.h = spriteJson.at("textureExtent").at("h");

    // Add the Y offset.
    sprite.yOffset = spriteJson.at("yOffset");

    // Add collisionEnabled.
    sprite.collisionEnabled = spriteJson.at("collisionEnabled");

    // Add modelBoundsID.
    sprite.modelBoundsID = spriteJson.at("modelBoundsID");

    // Add the custom model-space bounds.
    // Note: Even if this sprite uses a shared bounding box (modelBoundsID != 
    //       NULL_BOUNDING_BOX_ID), we want to save customModelBounds to give 
    //       the user a starting point if they switch to it.
    sprite.customModelBounds.minX = spriteJson.at("modelBounds").at("minX");
    sprite.customModelBounds.maxX = spriteJson.at("modelBounds").at("maxX");
    sprite.customModelBounds.minY = spriteJson.at("modelBounds").at("minY");
    sprite.customModelBounds.maxY = spriteJson.at("modelBounds").at("maxY");
    sprite.customModelBounds.minZ = spriteJson.at("modelBounds").at("minZ");
    sprite.customModelBounds.maxZ = spriteJson.at("modelBounds").at("maxZ");

    return true;
}

bool SpriteModel::spriteNameIsUnique(SpriteID spriteID,
                                     const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& spritePair : spriteMap) {
        SpriteID idToCheck{spritePair.first};
        const EditorSprite& sprite{spritePair.second};

        if ((idToCheck != spriteID) && (displayName == sprite.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

} // End namespace ResourceImporter
} // End namespace AM
