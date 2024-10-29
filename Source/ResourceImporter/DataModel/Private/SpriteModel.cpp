#include "SpriteModel.h"
#include "DataModel.h"
#include "GraphicSetModel.h"
#include "EntityGraphicSetModel.h"
#include "SpriteID.h"
#include "StringTools.h"
#include "SpriteTools.h"
#include "SDLHelpers.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <SDL_render.h>
#include <SDL_image.h>
#include <filesystem>

namespace AM
{
namespace ResourceImporter
{
SpriteModel::SpriteModel(DataModel& inDataModel, SDL_Renderer* inSdlRenderer)
: dataModel{inDataModel}
, sdlRenderer{inSdlRenderer}
, spriteSheetMap{}
, spriteMap{}
, sheetIDPool{MAX_SPRITE_SHEETS}
, spriteIDPool{MAX_SPRITES}
, errorString{}
, sheetAddedSig{}
, sheetRemovedSig{}
, spriteRemovedSig{}
, spriteSheetDisplayNameChangedSig{}
, spriteDisplayNameChangedSig{}
, spriteModelBoundsIDChangedSig{}
, spriteCustomModelBoundsChangedSig{}
, spriteCollisionEnabledChangedSig{}
, sheetAdded{sheetAddedSig}
, sheetRemoved{sheetRemovedSig}
, spriteAdded{spriteAddedSig}
, spriteRemoved{spriteRemovedSig}
, spriteSheetDisplayNameChanged{spriteSheetDisplayNameChangedSig}
, spriteDisplayNameChanged{spriteDisplayNameChangedSig}
, spriteModelBoundsIDChanged{spriteModelBoundsIDChangedSig}
, spriteCustomModelBoundsChanged{spriteCustomModelBoundsChangedSig}
, spriteCollisionEnabledChanged{spriteCollisionEnabledChangedSig}
, spriteStageOriginChanged{spriteStageOriginChangedSig}
{
    // Reserve the null sprite and sheet's ID (the engine provides it in code, 
    // so we don't need it in the json).
    spriteIDPool.reserveID();
    sheetIDPool.reserveID();
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
        // Add this sheet's display name.
        EditorSpriteSheet& spriteSheet{sheetPair.second};
        json["spriteSheets"][i]["displayName"] = spriteSheet.displayName;

        // For each sprite in this sheet.
        for (std::size_t j = 0; j < spriteSheet.spriteIDs.size(); ++j) {
            // Add the display name.
            EditorSprite& sprite{spriteMap[spriteSheet.spriteIDs[j]]};
            json["spriteSheets"][i]["sprites"][j]["displayName"]
                = sprite.displayName;

            // Derive the string ID from the display name and add it.
            std::string stringID{};
            StringTools::deriveStringID(sprite.displayName, stringID);
            json["spriteSheets"][i]["sprites"][j]["stringID"] = stringID;

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

            // Add the stage origin.
            json["spriteSheets"][i]["sprites"][j]["stageX"]
                = sprite.stageOrigin.x;
            json["spriteSheets"][i]["sprites"][j]["stageY"]
                = sprite.stageOrigin.y;

            // Add collisionEnabled.
            json["spriteSheets"][i]["sprites"][j]["collisionEnabled"]
                = sprite.collisionEnabled;

            // Add modelBoundsID.
            // Note: The engine doesn't use this, but this editor needs it.
            json["spriteSheets"][i]["sprites"][j]["modelBoundsID"]
                = sprite.modelBoundsID;

            // Add the model-space bounds.
            // Note: This will either be a shared bounding box or a custom one 
            //       depending on modelBoundsID. In either case, we always save
            //       to modelBounds because that's what the engine uses. 
            const BoundingBox& spriteModelBounds{
                sprite.getModelBounds(dataModel.boundingBoxModel)}; 
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minX"]
                = spriteModelBounds.min.x;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxX"]
                = spriteModelBounds.max.x;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minY"]
                = spriteModelBounds.min.y;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxY"]
                = spriteModelBounds.max.y;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minZ"]
                = spriteModelBounds.min.z;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxZ"]
                = spriteModelBounds.max.z;
        }

        i++;
    }
}

void SpriteModel::exportSpriteSheetImages()
{
    // Delete all of the existing files in the SpriteSheets directory.
    std::string spriteSheetsPath{dataModel.getWorkingTexturesDir()
                                 + "SpriteSheets"};
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(spriteSheetsPath)) {
        std::filesystem::remove_all(entry.path());
    }

    // Generate a texture for each sprite sheet and save it to a png file.
    for (auto& [spriteSheetID, spriteSheet] : spriteSheetMap) {
        SDL_Texture* spriteSheetTexture{
            SpriteTools::generateSpriteSheetTexture(dataModel, spriteSheet)};

        std::string filePath{spriteSheetsPath};
        filePath += "/" + spriteSheet.displayName + ".png";
        if (!(SDLHelpers::savePng(filePath, sdlRenderer, spriteSheetTexture))) {
            LOG_INFO("Failed to generate image for sprite sheet: %s",
                     spriteSheet.displayName.c_str());
        }
    }
}

bool SpriteModel::addSpriteSheet()
{
    SpriteSheetID numericID{
        static_cast<SpriteSheetID>(sheetIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewSpriteSheet"};
    while (!spriteSheetNameIsUnique(numericID, displayName)) {
        displayName = "NewSpriteSheet" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new sprite sheet to the maps.
    spriteSheetMap.emplace(numericID,
                           EditorSpriteSheet{numericID, displayName});
    spriteSheetNameMap.emplace(displayName, numericID);

    // Signal the new sheet to the UI.
    EditorSpriteSheet& spriteSheet{spriteSheetMap.at(numericID)};
    sheetAddedSig.publish(numericID, spriteSheet);

    // Set the new sheet as the active library item.
    dataModel.setActiveSpriteSheet(numericID);

    return true;
}

void SpriteModel::remSpriteSheet(SpriteSheetID sheetID)
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
    spriteSheetNameMap.erase(sheetIt->second.displayName);
    spriteSheetMap.erase(sheetIt);

    // Signal that the sheet was erased.
    sheetRemovedSig.publish(sheetID);
}

bool SpriteModel::addSprite(const std::string& imageRelPath,
                            SpriteSheetID parentSheetID,
                            const std::string& stageOriginX,
                            const std::string& stageOriginY)
{
    // Get the file name from the image path (we use the file name as the 
    // sprite's display name).
    std::string displayName{StringTools::getFileNameNoExtension(imageRelPath)};

    // If the given name isn't unique, fail.
    if (!spriteNameIsUnique(NULL_SPRITE_ID, displayName)) {
        errorString = "Name is already in use.";
        return false;
    }

    // Get the sprite texture's size.
    std::string fullImagePath{dataModel.getWorkingIndividualSpritesDir()};
    fullImagePath += imageRelPath;
    SDL_Texture* texture{IMG_LoadTexture(sdlRenderer, fullImagePath.c_str())};
    if (!texture) {
        errorString = "Failed to load texture: ";
        errorString += fullImagePath.c_str();
        return false;
    }
    SDL_Rect textureExtent{};
    SDL_QueryTexture(texture, nullptr, nullptr,
                     &(textureExtent.w), &(textureExtent.h));
    SDL_DestroyTexture(texture);

    // Validate the stage origin.
    SDL_Point stageOrigin{};
    try {
        stageOrigin.x = std::stoi(stageOriginX);
        stageOrigin.y = std::stoi(stageOriginY);
    } catch (std::exception&) {
        errorString = "Stage origin X or Y is not a valid integer.";
        return false;
    }

    // If the sprite is going to be added to an existing animation, validate 
    // the texture size (all sprites in an animation must be the same size).
    if (getFrameNumber(displayName) != -1) {
        AnimationModel& animationModel{dataModel.animationModel};
        std::string_view animationName{deriveAnimationName(displayName)};
        if (const EditorAnimation
            * animation{animationModel.getAnimation(animationName)}) {
            // Sprite is going to be added to this animation. Validate size.
            for (const auto& frame : animation->frames) {
                const SDL_Rect& animTextureExtent{
                    frame.sprite.get().textureExtent};
                if ((textureExtent.w != animTextureExtent.w)
                    || (textureExtent.h != animTextureExtent.h)) {
                    errorString
                        = "All sprites in an animation must be the same size.";
                    return false;
                }
            }
        }
    }

    // Add the new sprite to the maps.
    SpriteID numericID{static_cast<SpriteID>(spriteIDPool.reserveID())};
    spriteMap.emplace(numericID,
                      EditorSprite{numericID, imageRelPath, displayName,
                                   textureExtent, stageOrigin});
    spriteNameMap.emplace(displayName, numericID);

    // Add the sprite to its parent sheet.
    EditorSpriteSheet& parentSheet{mgetSpriteSheet(parentSheetID)};
    parentSheet.spriteIDs.push_back(numericID);

    // Refresh the sheet to account for the new sprite.
    refreshSpriteSheet(parentSheet);

    // Signal the new sprite to the UI.
    EditorSprite& sprite{spriteMap.at(numericID)};
    spriteAddedSig.publish(numericID, sprite, parentSheetID);

    // Add the sprite to an animation if necessary.
    addSpriteToAnimationIfNecessary(sprite);

    return true;
}

void SpriteModel::remSprite(SpriteID spriteID)
{
    // Find the sprite in the map.
    auto spriteIt{spriteMap.find(spriteID)};
    if (spriteIt == spriteMap.end()) {
        LOG_FATAL("Invalid ID while removing sprite.");
    }

    // Find the sheet that contains this sprite and remove it.
    EditorSpriteSheet* parentSpriteSheet{nullptr};
    for (auto& [spriteSheetID, spriteSheet] : spriteSheetMap) {
        std::vector<int>& spriteIDs{spriteSheet.spriteIDs};
        for (auto it{spriteIDs.begin()}; it != spriteIDs.end(); ++it) {
            if (*it == spriteID) {
                spriteIDs.erase(it);
                parentSpriteSheet = &spriteSheet;
                break;
            }
        }

        // If we found the sprite, stop looking.
        if (parentSpriteSheet) {
            break;
        }
    }
    AM_ASSERT(parentSpriteSheet, "Sprite did not belong to a sprite sheet.");

    // Refresh the sheet to account for the removed sprite.
    refreshSpriteSheet(*parentSpriteSheet);

    // If the sprite is in an animation, remove it.
    remSpriteFromAnimationIfNecessary(spriteIt->second);

    // Clear this sprite from any sets that reference it.
    dataModel.graphicSetModel.removeGraphicIDFromSets(toGraphicID(spriteID));
    dataModel.entityGraphicSetModel.removeGraphicIDFromSets(
        toGraphicID(spriteID));

    // Erase the sprite.
    spriteNameMap.erase(spriteIt->second.displayName);
    spriteMap.erase(spriteIt);

    // Signal that the sprite was erased.
    spriteRemovedSig.publish(spriteID, parentSpriteSheet->numericID);
}

const EditorSpriteSheet&
    SpriteModel::getSpriteSheet(SpriteSheetID sheetID) const
{
    auto spriteSheetIt{spriteSheetMap.find(sheetID)};
    if (spriteSheetIt == spriteSheetMap.end()) {
        LOG_FATAL("Tried to get sprite sheet with invalid ID: %d",
                  sheetID);
    }

    return spriteSheetIt->second;
}

const EditorSprite& SpriteModel::getSprite(SpriteID spriteID) const
{
    auto spriteIt{spriteMap.find(spriteID)};
    if (spriteIt == spriteMap.end()) {
        LOG_FATAL("Tried to get sprite with invalid ID: %d", spriteID);
    }

    return spriteIt->second;
}

void SpriteModel::setSpriteSheetDisplayName(SpriteSheetID spriteSheetID,
                                            const std::string& newDisplayName)
{
    auto spriteSheetPair{spriteSheetMap.find(spriteSheetID)};
    if (spriteSheetPair == spriteSheetMap.end()) {
        LOG_FATAL("Tried to set name using invalid sprite sheet ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!spriteSheetNameIsUnique(spriteSheetID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    EditorSpriteSheet& spriteSheet{spriteSheetPair->second};
    spriteSheet.displayName = uniqueDisplayName;

    // Signal the change.
    spriteSheetDisplayNameChangedSig.publish(spriteSheetID,
                                             spriteSheet.displayName);
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

void SpriteModel::setSpriteStageOrigin(SpriteID spriteID,
                                       const SDL_Point& newStageOrigin)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set stageOrigin using invalid sprite ID.");
    }

    // Set the new stageOrigin and signal the change.
    EditorSprite& sprite{spritePair->second};
    sprite.stageOrigin = newStageOrigin;

    spriteStageOriginChangedSig.publish(spriteID, newStageOrigin);
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

EditorSpriteSheet& SpriteModel::mgetSpriteSheet(SpriteSheetID sheetID)
{
    auto spriteSheetIt{spriteSheetMap.find(sheetID)};
    if (spriteSheetIt == spriteSheetMap.end()) {
        LOG_FATAL("Tried to get sprite sheet with invalid ID: %d",
                  sheetID);
    }

    return spriteSheetIt->second;
}

EditorSprite& SpriteModel::mgetSprite(SpriteID spriteID)
{
    auto spriteIt{spriteMap.find(spriteID)};
    if (spriteIt == spriteMap.end()) {
        LOG_FATAL("Tried to get sprite with invalid ID: %d", spriteID);
    }

    return spriteIt->second;
}

bool SpriteModel::parseSpriteSheet(const nlohmann::json& sheetJson)
{
    int sheetID{static_cast<int>(sheetIDPool.reserveID())};
    spriteSheetMap.emplace(sheetID, EditorSpriteSheet{});
    EditorSpriteSheet& spriteSheet{spriteSheetMap[sheetID]};

    // If the display name isn't unique, fail.
    std::string displayName{sheetJson.at("displayName").get<std::string>()};
    if (!spriteSheetNameIsUnique(sheetID, displayName)) {
        errorString = "Sprite sheet display name isn't unique: ";
        errorString += spriteSheet.displayName.c_str();
        return false;
    }
    spriteNameMap.emplace(displayName, sheetID);

    // Add the display name.
    spriteSheet.displayName = displayName;

    // Add the texture size.
    spriteSheet.textureWidth = sheetJson.at("textureWidth");
    spriteSheet.textureHeight = sheetJson.at("textureHeight");

    // Parse every sprite in this sheet.
    for (auto& spriteJson : sheetJson.at("sprites").items()) {
        if (!parseSprite(spriteJson.value(), spriteSheet)) {
            return false;
        }
    }

    // Set the sheet size and each sprite's texture position.
    refreshSpriteSheet(spriteSheet);

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

    spriteMap.emplace(spriteID, EditorSprite{spriteID});
    spriteSheet.spriteIDs.push_back(spriteID);
    EditorSprite& sprite{spriteMap[spriteID]};

    // Add the image path.
    std::string imagePath{spriteJson.at("imagePath").get<std::string>()};
    sprite.imagePath = imagePath;

    // If the display name isn't unique, fail.
    std::string displayName{spriteJson.at("displayName").get<std::string>()};
    if (!spriteNameIsUnique(spriteID, displayName)) {
        errorString = "Sprite display name isn't unique: ";
        errorString += sprite.displayName.c_str();
        return false;
    }
    spriteNameMap.emplace(displayName, spriteID);

    // Add the display name.
    sprite.displayName = displayName;

    // Add this sprite's extent within the sprite sheet.
    sprite.textureExtent.x = spriteJson.at("textureExtent").at("x");
    sprite.textureExtent.y = spriteJson.at("textureExtent").at("y");
    sprite.textureExtent.w = spriteJson.at("textureExtent").at("w");
    sprite.textureExtent.h = spriteJson.at("textureExtent").at("h");

    // Add the stage origin. 
    sprite.stageOrigin.x = spriteJson.at("stageX");
    sprite.stageOrigin.y = spriteJson.at("stageY");
    
    // Add collisionEnabled.
    sprite.collisionEnabled = spriteJson.at("collisionEnabled");

    // Add modelBoundsID.
    sprite.modelBoundsID = spriteJson.at("modelBoundsID");

    // Default our custom bounds to the saved modelBounds, regardless of 
    // whether we use a shared bounding box or not.
    sprite.customModelBounds.min.x = spriteJson.at("modelBounds").at("minX");
    sprite.customModelBounds.max.x = spriteJson.at("modelBounds").at("maxX");
    sprite.customModelBounds.min.y = spriteJson.at("modelBounds").at("minY");
    sprite.customModelBounds.max.y = spriteJson.at("modelBounds").at("maxY");
    sprite.customModelBounds.min.z = spriteJson.at("modelBounds").at("minZ");
    sprite.customModelBounds.max.z = spriteJson.at("modelBounds").at("maxZ");

    return true;
}

bool SpriteModel::spriteSheetNameIsUnique(SpriteSheetID spriteSheetID,
                                          const std::string& displayName)
{
    // If the desired name is already in the map, and the owner of the name 
    // isn't the given ID, the name isn't unique.
    auto it{spriteSheetNameMap.find(displayName)};
    if ((it != spriteSheetNameMap.end()) && (it->second != spriteSheetID)) {
        return false;
    }

    // Name isn't in the map, or it's owned by the same ID.
    return true;
}

bool SpriteModel::spriteNameIsUnique(SpriteID spriteID,
                                     const std::string& displayName)
{
    // If the desired name is already in the map, and the owner of the name 
    // isn't the given ID, the name isn't unique.
    auto it{spriteNameMap.find(displayName)};
    if ((it != spriteNameMap.end()) && (it->second != spriteID)) {
        return false;
    }

    // Name isn't in the map, or it's owned by the same ID.
    return true;
}

void SpriteModel::refreshSpriteSheet(EditorSpriteSheet& spriteSheet)
{
    // Algorithm: For now, we just build a grid where each cell is large 
    //            enough to fit the largest sprite.
    //            Ideally, this would more intelligently pack the sprites to 
    //            minimize empty space.

    // If there are no sprites, exit early.
    if (spriteSheet.spriteIDs.size() == 0) {
        spriteSheet.textureWidth = 0;
        spriteSheet.textureHeight = 0;
        return;
    }

    // Determine the largest sprite size.
    int maxSpriteWidth{0};
    int maxSpriteHeight{0};
    for (SpriteID spriteID : spriteSheet.spriteIDs) {
        EditorSprite& sprite{mgetSprite(spriteID)};
        if (sprite.textureExtent.w > maxSpriteWidth) {
            maxSpriteWidth = sprite.textureExtent.w;
        }
        if (sprite.textureExtent.h > maxSpriteHeight) {
            maxSpriteHeight = sprite.textureExtent.h;
        }
    }

    // Find the power of two that our texture's side lengths should equal, in 
    // order to fit all of the sprites (assuming a square grid).
    int spriteCount{static_cast<int>(spriteSheet.spriteIDs.size())};
    int gridSideLength{0};
    for (int powerOfTwo{0}; true; ++powerOfTwo) {
        gridSideLength = static_cast<int>(std::pow(2, powerOfTwo));

        // Determine how many sprites can fit with this side length.
        int spritesPerRow{gridSideLength / maxSpriteWidth};
        int spritesPerColumn{gridSideLength / maxSpriteHeight};
        if ((spritesPerRow < 1) || (spritesPerColumn < 1)) {
            // Can't fit any, continue to the next power of two.
            continue;
        }

        // If this power of two can fit all of the sprites, set it as our 
        // texture size.
        int gridCellCount{spritesPerRow * spritesPerColumn};
        if (gridCellCount >= spriteCount) {
            spriteSheet.textureWidth = gridSideLength;
            spriteSheet.textureHeight = gridSideLength;

            // If we're only going to fill half of the sheet or less, halve the 
            // height to save space.
            if (spriteCount <= (gridCellCount / 2)) {
                if (spriteCount == 1) {
                    spriteSheet.textureWidth /= 2;
                }
                else {
                    spriteSheet.textureHeight /= 2;
                }
            }
            break;
        }
    }

    // Place the sprites.
    int gridX{0};
    int gridY{0};
    for (SpriteID spriteID : spriteSheet.spriteIDs) {
        EditorSprite& sprite{mgetSprite(spriteID)};

        sprite.textureExtent.x = (gridX * maxSpriteWidth);
        sprite.textureExtent.y = (gridY * maxSpriteHeight);

        // If the next sprite would go past the edge of the grid, wrap to the 
        // next row.
        gridX++;
        if ((gridX * maxSpriteWidth) >= gridSideLength) {
            gridX = 0;
            gridY++;
        }
    }
}

void SpriteModel::addSpriteToAnimationIfNecessary(const EditorSprite& sprite)
{
    // If the sprite's display name contains a frame number, add it to an 
    // animation.
    // Note: Since we don't want to re-arrange someone's animation if they've 
    //       changed the order, we just add the sprite to the end. If we're 
    //       bulk adding sprites to an animation, the file system should serve 
    //       them to us in order anyway.
    if (getFrameNumber(sprite.displayName) != -1) {
        // Name contains a frame number. Derive the associated animation's name.
        AnimationModel& animationModel{dataModel.animationModel};
        std::string_view animationName{deriveAnimationName(sprite.displayName)};

        // Add this frame to the end of the animation.
        const EditorAnimation& animation{
            animationModel.addOrGetAnimation(animationName)};
        animationModel.addAnimationFrame(animation.numericID, sprite);
    }
}

void SpriteModel::remSpriteFromAnimationIfNecessary(const EditorSprite& sprite)
{
    // If the sprite's display name contains a frame number, remove it from the
    // animation.
    if (getFrameNumber(sprite.displayName) != -1) {
        // Name contains a frame number. Get the animation.
        std::string_view animationName{deriveAnimationName(sprite.displayName)};
        AnimationModel& animationModel{dataModel.animationModel};
        if (const EditorAnimation
            * animation{animationModel.getAnimation(animationName)}) {
            // Find which frame the sprite is in.
            // Note: This may be different than the frame number in the sprite's
            //       name, since frames can be reordered.
            int frameNumber{-1};
            for (const auto& frame : animation->frames) {
                if (frame.sprite.get().numericID == sprite.numericID) {
                    frameNumber = frame.frameNumber;
                    break;
                }
            }
            AM_ASSERT(frameNumber != -1, "Failed to find sprite in animation.");

            // Remove the sprite from the animation frame.
            dataModel.animationModel.clearAnimationFrame(
                animation->numericID, static_cast<Uint8>(frameNumber));

            // If the animation is now empty, delete it.
            if (animation->frames.empty()) {
                dataModel.animationModel.remAnimation(animation->numericID);
            }
        }
        else {
            AM_ASSERT(false, "Animation did not exist when expected.");
        }
    }
}

int SpriteModel::getFrameNumber(const std::string& displayName)
{
    // If the sprite's name ends in "_n" where n is a positive number, return 
    // it.
    if (auto pos{displayName.find_last_of('_')}; pos != displayName.npos) {
        // Get the string following the '_' and convert it to an integer.
        std::string frameNumberString{displayName.substr(pos + 1)};
        int frameNumber{-1};
        try {
            frameNumber = std::stoi(frameNumberString);
        } catch (std::exception&) {
            // The string following the '_' was not a valid number. Return
            // early.
            return -1;
        }

        // Check that the integer is valid.
        if (frameNumber < 0) {
            return -1;
        }

        return frameNumber;
    }

    return -1;
}

std::string_view
    SpriteModel::deriveAnimationName(std::string_view spriteDisplayName)
{
    return spriteDisplayName.substr(0, spriteDisplayName.find_last_of('_'));
}

} // End namespace ResourceImporter
} // End namespace AM
