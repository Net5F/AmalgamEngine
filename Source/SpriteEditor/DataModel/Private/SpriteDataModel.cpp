#include "SpriteDataModel.h"
#include "Log.h"
#include "Ignore.h"
#include "AUI/Core.h"
#include "nlohmann/json.hpp"

#include <SDL_render.h>
#include <SDL_image.h>

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace AM
{
namespace SpriteEditor
{
SpriteDataModel::SpriteDataModel(SDL_Renderer* inSdlRenderer)
: sheetAdded{sheetAddedSig}
, sheetRemoved{sheetRemovedSig}
, spriteRemoved{spriteRemovedSig}
, activeSpriteChanged{activeSpriteChangedSig}
, spriteDisplayNameChanged{spriteDisplayNameChangedSig}
, spriteCollisionEnabledChanged{spriteCollisionEnabledChangedSig}
, spriteModelBoundsChanged{spriteModelBoundsChangedSig}
, sdlRenderer{inSdlRenderer}
, workingFilePath{""}
, workingTexturesDir{""}
, activeSpriteID{INVALID_SPRITE_ID}
, sheetIDPool{MAX_SPRITE_SHEETS}
, spriteIDPool{MAX_SPRITES}
{
    // Burn '0' since we reserve it for our invalid ID.
    spriteIDPool.reserveID();
}

std::string SpriteDataModel::create(const std::string& fullPath)
{
    // If a SpriteData.json already exists at the given path, return false.
    workingFilePath = fullPath;
    workingFilePath += "/SpriteData.json";
    if (std::filesystem::exists(workingFilePath)) {
        workingFilePath = "";
        return "SpriteData.json file already exists at the selected path.";
    }

    // Create the file.
    std::ofstream workingFile(workingFilePath, std::ios::app);
    workingFile.close();

    // Set the working directory.
    if (!setWorkingTexturesDir()) {
        return "Failed to create Resources directory.";
    }

    // Save our empty model structure.
    save();

    return "";
}

std::string SpriteDataModel::load(const std::string& fullPath)
{
    // Open the file.
    std::ifstream workingFile(fullPath);
    if (workingFile.is_open()) {
        workingFilePath = fullPath;

        // Set the working directory.
        if (!setWorkingTexturesDir()) {
            return "Failed to create Resources directory.";
        }
    }
    else {
        return "File failed to open.";
    }

    // Parse the json file to fill our data model.
    try {
        // Parse the file into a json structure.
        nlohmann::json json = nlohmann::json::parse(workingFile, nullptr);

        // For every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            unsigned int sheetID{sheetIDPool.reserveID()};
            spriteSheetMap.emplace(sheetID, SpriteSheet{});
            SpriteSheet& spriteSheet{spriteSheetMap[sheetID]};

            // Add this sheet's relative path.
            spriteSheet.relPath
                = sheetJson.value()["relPath"].get<std::string>();
            std::string resultString{validateRelPath(spriteSheet.relPath)};
            if (resultString != "") {
                workingFile.close();
                spriteSheetMap.clear();
                spriteMap.clear();
                return resultString;
            }

            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                unsigned int spriteID{spriteIDPool.reserveID()};
                spriteMap.emplace(spriteID, Sprite{spriteSheet.relPath});
                spriteSheet.spriteIDs.push_back(spriteID);
                Sprite& sprite{spriteMap[spriteID]};

                // If the display name isn't unique, fail.
                std::string displayName{
                    spriteJson.value()["displayName"].get<std::string>()};
                if (!spriteNameIsUnique(spriteID, displayName)) {
                    std::string returnString{
                        "Sprite display name isn't unique: "};
                    returnString += sprite.displayName.c_str();

                    workingFile.close();
                    spriteSheetMap.clear();
                    spriteMap.clear();
                    return returnString;
                }

                // Add the display name.
                sprite.displayName = displayName;

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

                // Add collisionEnabled.
                sprite.collisionEnabled = spriteJson.value()["collisionEnabled"];

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
            }

            // Signal the new sheet to the UI.
            sheetAddedSig.publish(sheetID, spriteSheet);
        }
    } catch (nlohmann::json::type_error& e) {
        workingFile.close();
        spriteSheetMap.clear();
        spriteMap.clear();
        std::string failureString{"Parse failure - "};
        failureString += e.what();
        return failureString;
    }

    return "";
}

void SpriteDataModel::save()
{
    // Create a new json structure to fill.
    nlohmann::json json;
    json["spriteSheets"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int nextNumericId{0};
    int i{0};
    for (auto& sheetPair : spriteSheetMap) {
        // Add this sheet's relative path.
        SpriteSheet& spriteSheet{sheetPair.second};
        json["spriteSheets"][i]["relPath"] = spriteSheet.relPath;

        // For each sprite in this sheet.
        for (unsigned int j = 0; j < spriteSheet.spriteIDs.size(); ++j) {
            // Add the display name.
            Sprite& sprite{spriteMap[spriteSheet.spriteIDs[j]]};
            json["spriteSheets"][i]["sprites"][j]["displayName"]
                = sprite.displayName;

            // Derive the string ID from the display name and add it.
            json["spriteSheets"][i]["sprites"][j]["stringID"]
                = deriveStringId(sprite.displayName);

            // Add the numeric ID.
            json["spriteSheets"][i]["sprites"][j]["numericID"] = nextNumericId;
            nextNumericId++;

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

            // Add the model-space bounds.
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minX"]
                = sprite.modelBounds.minX;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxX"]
                = sprite.modelBounds.maxX;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minY"]
                = sprite.modelBounds.minY;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxY"]
                = sprite.modelBounds.maxY;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["minZ"]
                = sprite.modelBounds.minZ;
            json["spriteSheets"][i]["sprites"][j]["modelBounds"]["maxZ"]
                = sprite.modelBounds.maxZ;
        }

        i++;
    }

    // Write the json to our working file.
    std::ofstream workingFile(workingFilePath, std::ios::trunc);
    if (!(workingFile.is_open())) {
        LOG_FATAL("File failed to open: %s.", workingFilePath.c_str());
    }

    std::string jsonDump{json.dump(4)};
    workingFile << jsonDump;
}

std::string SpriteDataModel::addSpriteSheet(const std::string& relPath,
                                            const std::string& spriteWidth,
                                            const std::string& spriteHeight,
                                            const std::string& yOffset,
                                            const std::string& baseName)
{
    /* Validate the data. */
    // Check if we already have the given sheet.
    for (const auto& sheetPair : spriteSheetMap) {
        if (sheetPair.second.relPath == relPath) {
            return "Error: Path conflicts with existing sprite sheet.";
        }
    }

    // Append the texture directory to the given relative path.
    std::string fullPath{workingTexturesDir};
    fullPath += relPath;

    // Validate that the file at the given path is a valid texture.
    int sheetWidth{0};
    int sheetHeight{0};
    SDL_Texture* sheetTexture{IMG_LoadTexture(sdlRenderer, fullPath.c_str())};
    if (sheetTexture == nullptr) {
        std::string errorString{
            "Error: File at given path is not a valid image. Path: "};
        errorString += workingTexturesDir;
        errorString += relPath;
        return errorString;
    }
    else {
        // Save the texture size for later.
        SDL_QueryTexture(sheetTexture, nullptr, nullptr, &sheetWidth,
                         &sheetHeight);

        // We don't need the actual texture right now, destroy it.
        SDL_DestroyTexture(sheetTexture);
    }

    // Validate the width/height/yOffset.
    int spriteWidthI{0};
    int spriteHeightI{0};
    int yOffsetI{0};
    try {
        spriteWidthI = std::stoi(spriteWidth);
        spriteHeightI = std::stoi(spriteHeight);
        yOffsetI = std::stoi(yOffset);
    } catch (std::exception& e) {
        ignore(e);
        return "Error: Width, height, or Y offset is not a valid integer.";
    }

    // Validate the size of the texture.
    if ((spriteWidthI > sheetWidth) || (spriteHeightI > sheetHeight)) {
        return "Error: Sheet must be larger than sprite size.";
    }

    /* Add the sprite sheet and sprites. */
    unsigned int sheetID{sheetIDPool.reserveID()};
    spriteSheetMap.emplace(sheetID, SpriteSheet{relPath, {}});

    // For each sprite in this texture.
    SpriteSheet& spriteSheet{spriteSheetMap[sheetID]};
    int spriteCount{0};
    for (int y = 0; y <= (sheetHeight - spriteHeightI); y += spriteHeightI) {
        for (int x = 0; x <= (sheetWidth - spriteWidthI); x += spriteWidthI) {
            // Build the sprite's display name (baseName_count).
            std::string displayName{baseName};
            displayName += std::to_string(spriteCount);

            // Find the sprite's extent within the sheet texture.
            SDL_Rect textureExtent{x, y, spriteWidthI, spriteHeightI};

            // Default to a non-0 bounding box so it's easier to click.
            static BoundingBox defaultBox{0, 20, 0, 20, 0, 20};

            // Add the sprite to the map and sheet.
            unsigned int spriteID{spriteIDPool.reserveID()};
            spriteMap.emplace(spriteID, Sprite{spriteSheet.relPath, displayName,
                                               textureExtent, yOffsetI, true,
                                               defaultBox});
            spriteSheet.spriteIDs.push_back(spriteID);

            // Increment the count (used for the display name).
            spriteCount++;
        }
    }

    // Signal the new sheet to the UI.
    sheetAddedSig.publish(sheetID, spriteSheet);

    return "";
}

void SpriteDataModel::remSpriteSheet(unsigned int sheetID)
{
    // Find the sheet in the map.
    auto sheetIt{spriteSheetMap.find(sheetID)};
    if (sheetIt == spriteSheetMap.end()) {
        LOG_FATAL("Invalid ID while removing sprite sheet.");
    }

    // Erase all of the sheet's sprites.
    for (unsigned int spriteID : sheetIt->second.spriteIDs) {
        remSprite(spriteID);
    }

    // Erase the sheet.
    spriteSheetMap.erase(sheetIt);

    // Signal that the sheet was erased.
    sheetRemovedSig.publish(sheetID);
}

void SpriteDataModel::remSprite(unsigned int spriteID)
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
}

const Sprite& SpriteDataModel::getSprite(unsigned int spriteID)
{
    auto spriteIt{spriteMap.find(spriteID)};
    if (spriteIt == spriteMap.end()) {
        LOG_FATAL("Tried to get sprite with invalid ID.");
    }

    return spriteIt->second;
}

const std::string& SpriteDataModel::getWorkingTexturesDir()
{
    return workingTexturesDir;
}

void SpriteDataModel::setActiveSprite(unsigned int newActiveSpriteID)
{
    auto spritePair{spriteMap.find(newActiveSpriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set active sprite to invalid ID.");
    }

    // Set the active sprite and signal it to the UI.
    activeSpriteID = newActiveSpriteID;
    activeSpriteChangedSig.publish(activeSpriteID, spritePair->second);
}

void SpriteDataModel::setSpriteDisplayName(unsigned int spriteID,
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

    Sprite& sprite{spritePair->second};
    sprite.displayName = uniqueDisplayName;

    // Signal the change.
    spriteDisplayNameChangedSig.publish(spriteID, sprite.displayName);
}

void SpriteDataModel::setSpriteCollisionEnabled(unsigned int spriteID,
                                              bool newCollisionEnabled)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set collisionEnabled using invalid sprite ID.");
    }

    // Set the new collisionEnabled and signal the change.
    Sprite& sprite{spritePair->second};
    sprite.collisionEnabled = newCollisionEnabled;

    spriteCollisionEnabledChangedSig.publish(spriteID, newCollisionEnabled);
}

void SpriteDataModel::setSpriteModelBounds(unsigned int spriteID,
                                           const BoundingBox& newModelBounds)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set boundingBox using invalid sprite ID.");
    }

    // Set the new model bounds and signal the change.
    Sprite& sprite{spritePair->second};
    sprite.modelBounds = newModelBounds;

    spriteModelBoundsChangedSig.publish(spriteID, newModelBounds);
}

std::string SpriteDataModel::validateRelPath(const std::string& relPath)
{
    // Construct the file path.
    std::filesystem::path filePath{workingTexturesDir};
    filePath /= relPath;

    // Check if the file exists.
    if (std::filesystem::exists(filePath)) {
        return "";
    }
    else {
        // File doesn't exist, return an error string.
        std::string returnString{"File not found at Assets/Textures/"};
        returnString += relPath;
        return returnString;
    }
}

bool SpriteDataModel::setWorkingTexturesDir()
{
    // Construct the assets dir path.
    std::filesystem::path texturesDirPath{workingFilePath};
    texturesDirPath = texturesDirPath.parent_path();
    texturesDirPath /= "Assets/Textures/";

    // Check if the textures dir exists.
    if (!std::filesystem::exists(texturesDirPath)) {
        // Directory doesn't exist, create it.
        try {
            std::filesystem::create_directories(texturesDirPath);
        } catch (std::filesystem::filesystem_error& e) {
            LOG_INFO("Failed to create Textures directory. Path: %s, Error: %s",
                     texturesDirPath.string().c_str(), e.what());
            return false;
        }
    }

    // Save the path as a UTF-8 string.
    // Note: We've had trouble consistently getting a UTF-8 string from a
    //       fs::path. If this breaks on some platform, we'll have to revisit.
    workingTexturesDir = texturesDirPath.generic_string();

    return true;
}

std::string SpriteDataModel::deriveStringId(const std::string& displayName)
{
    // Make the string all lowercase.
    std::string stringID{displayName};
    std::transform(stringID.begin(), stringID.end(), stringID.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Replace spaces with underscores.
    std::replace(stringID.begin(), stringID.end(), ' ', '_');

    return stringID;
}

bool SpriteDataModel::spriteNameIsUnique(unsigned int spriteID,
                                         const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& spritePair : spriteMap) {
        unsigned int idToCheck{spritePair.first};
        const Sprite& sprite{spritePair.second};

        if ((idToCheck != spriteID) && (displayName == sprite.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

} // End namespace SpriteEditor
} // End namespace AM
