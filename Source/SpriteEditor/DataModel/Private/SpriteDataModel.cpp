#include "SpriteDataModel.h"
#include "EmptySpriteID.h"
#include "Log.h"
#include "Ignore.h"
#include "AMAssert.h"
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
, activeLibraryItemChanged{activeLibraryItemChangedSig}
, spriteDisplayNameChanged{spriteDisplayNameChangedSig}
, spriteCollisionEnabledChanged{spriteCollisionEnabledChangedSig}
, spriteModelBoundsChanged{spriteModelBoundsChangedSig}
, floorAdded{floorAddedSig}
, floorCoveringAdded{floorCoveringAddedSig}
, wallAdded{wallAddedSig}
, objectAdded{objectAddedSig}
, spriteSetRemoved{spriteSetRemovedSig}
, spriteSetSlotChanged{spriteSetSlotChangedSig}
, spriteSetDisplayNameChanged{spriteSetDisplayNameChangedSig}
, sdlRenderer{inSdlRenderer}
, workingFilePath{""}
, workingTexturesDir{""}
, sheetIDPool{MAX_SPRITE_SHEETS}
, spriteIDPool{MAX_SPRITES}
, floorIDPool{SDL_MAX_UINT16}
, floorCoveringIDPool{SDL_MAX_UINT16}
, wallIDPool{SDL_MAX_UINT16}
, objectIDPool{SDL_MAX_UINT16}
{
}

bool SpriteDataModel::create(const std::string& fullPath)
{
    // If a SpriteData.json already exists at the given path, return false.
    workingFilePath = fullPath;
    workingFilePath += "/SpriteData.json";
    if (std::filesystem::exists(workingFilePath)) {
        workingFilePath = "";
        errorString = "SpriteData.json file already exists at the selected path.";
        return false;
    }

    // Create the file.
    std::ofstream workingFile(workingFilePath, std::ios::app);
    workingFile.close();

    // Set the working directory.
    if (!setWorkingTexturesDir()) {
        errorString = "Failed to create Resources directory.";
        return false;
    }

    // Save our empty model structure.
    save();

    return true;
}

bool SpriteDataModel::load(const std::string& fullPath)
{
    // Open the file.
    std::ifstream workingFile(fullPath);
    if (workingFile.is_open()) {
        workingFilePath = fullPath;

        // Set the working directory.
        if (!setWorkingTexturesDir()) {
            errorString = "Failed to create Resources directory.";
            return false;
        }
    }
    else {
        errorString = "File failed to open.";
        return false;
    }

    // Parse the json file to fill our data model.
    try {
        // Parse the file into a json structure.
        nlohmann::json json = nlohmann::json::parse(workingFile, nullptr);

        // Iterate every sprite sheet and add all their sprites.
        for (auto& sheetJson : json.at("spriteSheets").items()) {
            if (!parseSpriteSheet(sheetJson.value())) {
                resetModelState();
                return false;
            }
        }

        // Add each type of sprite set.
        for (auto& floorJson : json.at("floors").items()) {
            if (!parseFloorSpriteSet(floorJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& floorCoveringJson : json.at("floorCoverings").items()) {
            if (!parseFloorCoveringSpriteSet(floorCoveringJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& wallJson : json.at("walls").items()) {
            std::string resultString{};
            if (!parseWallSpriteSet(wallJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& objectJson : json.at("objects").items()) {
            if (!parseObjectSpriteSet(objectJson.value())) {
                resetModelState();
                return false;
            }
        }
    } catch (nlohmann::json::type_error& e) {
        resetModelState();
        errorString = "Parse failure - ";
        errorString += e.what();
        return false;
    }

    return "";
}

void SpriteDataModel::save()
{
    // Save each part of the model to a json object.
    nlohmann::json json;
    saveSpriteSheets(json);
    saveFloors(json);
    saveFloorCoverings(json);
    saveWalls(json);
    saveObjects(json);

    // Write the json to our working file.
    std::ofstream workingFile(workingFilePath, std::ios::trunc);
    if (!(workingFile.is_open())) {
        LOG_FATAL("File failed to open: %s.", workingFilePath.c_str());
    }

    std::string jsonDump{json.dump(4)};
    workingFile << jsonDump;
}

bool SpriteDataModel::addSpriteSheet(const std::string& relPath,
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

    // Append the texture directory to the given relative path.
    std::string fullPath{workingTexturesDir};
    fullPath += relPath;

    // Validate that the file at the given path is a valid texture.
    int sheetWidth{0};
    int sheetHeight{0};
    SDL_Texture* sheetTexture{IMG_LoadTexture(sdlRenderer, fullPath.c_str())};
    if (sheetTexture == nullptr) {
        errorString = "Error: File at given path is not a valid image. Path: ";
        errorString += workingTexturesDir;
        errorString += relPath;
        return false;
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
            static BoundingBox defaultBox{0, 20, 0, 20, 0, 20};

            // Add the sprite to the map and sheet.
            int spriteID{static_cast<int>(spriteIDPool.reserveID())};
            spriteMap.emplace(spriteID, EditorSprite{spriteID, spriteSheet.relPath,
                                               displayName, textureExtent,
                                               yOffsetI, true, defaultBox});
            spriteSheet.spriteIDs.push_back(spriteID);

            // Increment the count (used for the display name).
            spriteCount++;
        }
    }

    // Signal the new sheet to the UI.
    sheetAddedSig.publish(sheetID, spriteSheet);

    return true;
}

void SpriteDataModel::remSpriteSheet(int sheetID)
{
    // Find the sheet in the map.
    auto sheetIt{spriteSheetMap.find(sheetID)};
    if (sheetIt == spriteSheetMap.end()) {
        LOG_FATAL("Invalid ID while removing sprite sheet.");
    }

    // Erase all of the sheet's sprites.
    for (int spriteID : sheetIt->second.spriteIDs) {
        remSprite(spriteID);
    }

    // Erase the sheet.
    spriteSheetMap.erase(sheetIt);

    // Signal that the sheet was erased.
    sheetRemovedSig.publish(sheetID);
}

void SpriteDataModel::remSprite(int spriteID)
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
    removeSpriteIDFromSets(spriteID);
}

const EditorSprite& SpriteDataModel::getSprite(int spriteID)
{
    auto spriteIt{spriteMap.find(spriteID)};
    if (spriteIt == spriteMap.end()) {
        LOG_FATAL("Tried to get sprite with invalid ID: %d", spriteID);
    }

    return spriteIt->second;
}

bool SpriteDataModel::addFloor()
{
    Uint16 numericID{static_cast<Uint16>(floorIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewFloor"};
    while (
        !spriteSetNameIsUnique<EditorFloorSpriteSet>(numericID, displayName)) {
        displayName = "NewFloor" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty sprite set to the map.
    std::array<int, 1> spriteIDs{EMPTY_SPRITE_ID};
    floorMap.emplace(numericID,
                     EditorFloorSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorSpriteSet& spriteSet{floorMap[numericID]};
    floorAddedSig.publish(numericID, spriteSet);

    // Set the new sprite set as the active library item.
    setActiveSpriteSet(SpriteSet::Type::Floor, numericID);

    return true;
}

bool SpriteDataModel::addFloorCovering()
{
    Uint16 numericID{static_cast<Uint16>(floorCoveringIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewFloorCovering"};
    while (!spriteSetNameIsUnique<EditorFloorCoveringSpriteSet>(numericID,
                                                                displayName)) {
        displayName = "NewFloorCovering" + std::to_string(nameCount);
    }

    // Add the new, empty sprite set to the map.
    std::array<int, Rotation::Direction::Count> spriteIDs{
        EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID,
        EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID};
    floorCoveringMap.emplace(numericID, EditorFloorCoveringSpriteSet{
                                            numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorCoveringSpriteSet& spriteSet{floorCoveringMap[numericID]};
    floorCoveringAddedSig.publish(numericID, spriteSet);

    // Set the new sprite as the active library item.
    setActiveSpriteSet(SpriteSet::Type::FloorCovering, numericID);

    return true;
}

bool SpriteDataModel::addWall()
{
    Uint16 numericID{static_cast<Uint16>(wallIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewWall"};
    while (
        !spriteSetNameIsUnique<EditorWallSpriteSet>(numericID, displayName)) {
        displayName = "NewWall" + std::to_string(nameCount);
    }

    // Add the new, empty sprite set to the map.
    std::array<int, Wall::Type::Count> spriteIDs{
        EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID};
    wallMap.emplace(numericID,
                     EditorWallSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorWallSpriteSet& spriteSet{wallMap[numericID]};
    wallAddedSig.publish(numericID, spriteSet);

    // Set the new sprite as the active library item.
    setActiveSpriteSet(SpriteSet::Type::Wall, numericID);

    return true;
}

bool SpriteDataModel::addObject()
{
    Uint16 numericID{static_cast<Uint16>(objectIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewObject"};
    while (
        !spriteSetNameIsUnique<EditorObjectSpriteSet>(numericID, displayName)) {
        displayName = "NewObject" + std::to_string(nameCount);
    }

    // Add the new, empty sprite set to the map.
    std::array<int, Rotation::Direction::Count> spriteIDs{
        EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID,
        EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID};
    objectMap.emplace(numericID,
                      EditorObjectSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorObjectSpriteSet& spriteSet{objectMap[numericID]};
    objectAddedSig.publish(numericID, spriteSet);

    // Set the new sprite as the active library item.
    setActiveSpriteSet(SpriteSet::Type::Object, numericID);

    return true;
}

void SpriteDataModel::remFloor(Uint16 floorID)
{
    // Find the floor in the map.
    auto floorIt{floorMap.find(floorID)};
    if (floorIt == floorMap.end()) {
        LOG_FATAL("Invalid ID while removing floor.");
    }

    // Erase the floor.
    floorMap.erase(floorIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::Floor, floorID);
}

void SpriteDataModel::remFloorCovering(Uint16 floorCoveringID)
{
    // Find the floor covering in the map.
    auto floorCoveringIt{floorCoveringMap.find(floorCoveringID)};
    if (floorCoveringIt == floorCoveringMap.end()) {
        LOG_FATAL("Invalid ID while removing floor covering.");
    }

    // Erase the floor covering.
    floorCoveringMap.erase(floorCoveringIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::FloorCovering,
                                floorCoveringID);
}

void SpriteDataModel::remWall(Uint16 wallID)
{
    // Find the wall in the map.
    auto wallIt{wallMap.find(wallID)};
    if (wallIt == wallMap.end()) {
        LOG_FATAL("Invalid ID while removing wall.");
    }

    // Erase the sprite.
    wallMap.erase(wallIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::Wall, wallID);
}

void SpriteDataModel::remObject(Uint16 objectID)
{
    // Find the floor in the map.
    auto objectIt{objectMap.find(objectID)};
    if (objectIt == objectMap.end()) {
        LOG_FATAL("Invalid ID while removing object.");
    }

    // Erase the sprite.
    objectMap.erase(objectIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::Object, objectID);
}

void SpriteDataModel::setActiveSprite(int newActiveSpriteID)
{
    auto spritePair{spriteMap.find(newActiveSpriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set active sprite to invalid ID.");
    }

    // Signal the active sprite to the UI.
    activeLibraryItemChangedSig.publish(spritePair->second);
}

void SpriteDataModel::setActiveSpriteSet(SpriteSet::Type type, Uint16 newActiveSpriteSetID)
{
    // Match the sprite set type, then:
    //   1. Check if a set of that type with the given ID exists.
    //   2. If so, signal it out.
    switch (type) {
        case SpriteSet::Type::Floor: {
            auto pair{floorMap.find(newActiveSpriteSetID)};
            if (pair != floorMap.end()) {
                activeLibraryItemChangedSig.publish(pair->second);
                return;
            }
            break;
        }
        case SpriteSet::Type::FloorCovering: {
            auto pair{floorCoveringMap.find(newActiveSpriteSetID)};
            if (pair != floorCoveringMap.end()) {
                activeLibraryItemChangedSig.publish(pair->second);
                return;
            }
            break;
        }
        case SpriteSet::Type::Wall: {
            auto pair{wallMap.find(newActiveSpriteSetID)};
            if (pair != wallMap.end()) {
                activeLibraryItemChangedSig.publish(pair->second);
                return;
            }
            break;
        }
        case SpriteSet::Type::Object: {
            auto pair{objectMap.find(newActiveSpriteSetID)};
            if (pair != objectMap.end()) {
                activeLibraryItemChangedSig.publish(pair->second);
                return;
            }
            break;
        }
        default: {
            LOG_FATAL("Unsupported sprite set type: %u", type);
        }
    }

    // If we reached here, no sprite set was found with the matching ID and 
    // type.
    LOG_FATAL("Tried to set active sprite set to invalid ID.");
}

void SpriteDataModel::setSpriteDisplayName(int spriteID,
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

void SpriteDataModel::setSpriteCollisionEnabled(int spriteID,
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

void SpriteDataModel::setSpriteModelBounds(int spriteID,
                                           const BoundingBox& newModelBounds)
{
    auto spritePair{spriteMap.find(spriteID)};
    if (spritePair == spriteMap.end()) {
        LOG_FATAL("Tried to set boundingBox using invalid sprite ID.");
    }

    // Set the new model bounds and signal the change.
    EditorSprite& sprite{spritePair->second};
    sprite.modelBounds = newModelBounds;

    spriteModelBoundsChangedSig.publish(spriteID, newModelBounds);
}

void SpriteDataModel::setSpriteSetDisplayName(SpriteSet::Type type, Uint16 spriteSetID,
    const std::string& newDisplayName)
{
    switch (type) {
        case SpriteSet::Type::Floor: {
            setSpriteSetDisplayName<EditorFloorSpriteSet>(type, spriteSetID,
                                                          newDisplayName);
            break;
        }
        case SpriteSet::Type::FloorCovering: {
            setSpriteSetDisplayName<EditorFloorCoveringSpriteSet>(
                type, spriteSetID, newDisplayName);
            break;
        }
        case SpriteSet::Type::Wall: {
            setSpriteSetDisplayName<EditorWallSpriteSet>(type, spriteSetID,
                                                         newDisplayName);
            break;
        }
        case SpriteSet::Type::Object: {
            setSpriteSetDisplayName<EditorObjectSpriteSet>(type, spriteSetID,
                                                           newDisplayName);
            break;
        }
        default: {
            LOG_FATAL("Invalid sprite set type.");
            break;
        }
    }
}

void SpriteDataModel::setSpriteSetSlot(SpriteSet::Type type, Uint16 spriteSetID,
    std::size_t index, int newSpriteID)
{
    switch (type) {
        case SpriteSet::Type::Floor: {
            setSpriteSetSlot<EditorFloorSpriteSet>(type, spriteSetID, index,
                                                   newSpriteID);
            break;
        }
        case SpriteSet::Type::FloorCovering: {
            setSpriteSetSlot<EditorFloorCoveringSpriteSet>(type, spriteSetID,
                                                           index, newSpriteID);
            break;
        }
        case SpriteSet::Type::Wall: {
            setSpriteSetSlot<EditorWallSpriteSet>(type, spriteSetID, index,
                                                  newSpriteID);
            break;
        }
        case SpriteSet::Type::Object: {
            setSpriteSetSlot<EditorObjectSpriteSet>(type, spriteSetID, index,
                                                    newSpriteID);
            break;
        }
        default: {
            LOG_FATAL("Invalid sprite set type.");
            break;
        }
    }
}

const std::string& SpriteDataModel::getWorkingTexturesDir()
{
    return workingTexturesDir;
}

const std::string& SpriteDataModel::getErrorString()
{
    return errorString;
}

bool SpriteDataModel::validateRelPath(const std::string& relPath)
{
    // Construct the file path.
    std::filesystem::path filePath{workingTexturesDir};
    filePath /= relPath;

    // Check if the file exists.
    if (std::filesystem::exists(filePath)) {
        return true;
    }
    else {
        errorString = "File not found at Assets/Textures/";
        errorString += relPath;
        return false;
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

std::string SpriteDataModel::deriveStringID(const std::string& displayName)
{
    // Make the string all lowercase.
    std::string stringID{displayName};
    std::transform(stringID.begin(), stringID.end(), stringID.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Replace spaces with underscores.
    std::replace(stringID.begin(), stringID.end(), ' ', '_');

    return stringID;
}

bool SpriteDataModel::parseSpriteSheet(const nlohmann::json& sheetJson)
{
    int sheetID{static_cast<int>(sheetIDPool.reserveID())};
    spriteSheetMap.emplace(sheetID, EditorSpriteSheet{});
    EditorSpriteSheet& spriteSheet{spriteSheetMap[sheetID]};

    // Add this sheet's relative path.
    spriteSheet.relPath
        = sheetJson.at("relPath").get<std::string>();
    if (!validateRelPath(spriteSheet.relPath)) {
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

bool SpriteDataModel::parseSprite(const nlohmann::json& spriteJson,
                     EditorSpriteSheet& spriteSheet)
{
    // Mark the sprite's ID as reserved so it doesn't get reused.
    int spriteID{spriteJson.at("numericID").get<int>()};
    spriteIDPool.markIDAsReserved(spriteID);

    spriteMap.emplace(spriteID,
                      EditorSprite{spriteID, spriteSheet.relPath});
    spriteSheet.spriteIDs.push_back(spriteID);
    EditorSprite& sprite{spriteMap[spriteID]};

    // If the display name isn't unique, fail.
    std::string displayName{
        spriteJson.at("displayName").get<std::string>()};
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

    // Add the model-space bounds.
    sprite.modelBounds.minX = spriteJson.at("modelBounds").at("minX");
    sprite.modelBounds.maxX = spriteJson.at("modelBounds").at("maxX");
    sprite.modelBounds.minY = spriteJson.at("modelBounds").at("minY");
    sprite.modelBounds.maxY = spriteJson.at("modelBounds").at("maxY");
    sprite.modelBounds.minZ = spriteJson.at("modelBounds").at("minZ");
    sprite.modelBounds.maxZ = spriteJson.at("modelBounds").at("maxZ");

    return true;
}

bool SpriteDataModel::parseFloorSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(floorIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    // Note: Floors just have 1 sprite, but the json uses an array in case we
    //       want to add variations in the future.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, 1> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorFloorSpriteSet>(spriteIDs[i],
                                                         displayName)) {
            errorString = "Floor display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    floorMap.emplace(numericID,
                     EditorFloorSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorSpriteSet& spriteSet{floorMap[numericID]};
    floorAddedSig.publish(numericID, spriteSet);

    return true;
}

bool SpriteDataModel::parseFloorCoveringSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(floorCoveringIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, Rotation::Direction::Count> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorFloorCoveringSpriteSet>(spriteIDs[i],
                                                                 displayName)) {
            errorString = "Floor covering display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    floorCoveringMap.emplace(numericID, EditorFloorCoveringSpriteSet{
                                            numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorCoveringSpriteSet& spriteSet{floorCoveringMap[numericID]};
    floorCoveringAddedSig.publish(numericID, spriteSet);

    return true;
}

bool SpriteDataModel::parseWallSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(wallIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, Wall::Type::Count> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorWallSpriteSet>(spriteIDs[i],
                                                        displayName)) {
            errorString = "Wall display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    wallMap.emplace(numericID,
                    EditorWallSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorWallSpriteSet& spriteSet{wallMap[numericID]};
    wallAddedSig.publish(numericID, spriteSet);

    return true;
}

bool SpriteDataModel::parseObjectSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(objectIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, Rotation::Direction::Count> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorObjectSpriteSet>(spriteIDs[i],
                                                          displayName)) {
            errorString = "Object display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    objectMap.emplace(numericID,
                      EditorObjectSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorObjectSpriteSet& spriteSet{objectMap[numericID]};
    objectAddedSig.publish(numericID, spriteSet);

    return true;
}

bool SpriteDataModel::spriteNameIsUnique(int spriteID,
                                         const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& spritePair : spriteMap) {
        int idToCheck{spritePair.first};
        const EditorSprite& sprite{spritePair.second};

        if ((idToCheck != spriteID) && (displayName == sprite.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

template<typename T>
std::map<Uint16, T>& SpriteDataModel::getMapForSpriteSetType()
{
    std::map<Uint16, T>* map{nullptr};
    if constexpr (std::same_as<T, EditorFloorSpriteSet>) {
        return floorMap;
    }
    else if constexpr (std::same_as<T, EditorFloorCoveringSpriteSet>) {
        return floorCoveringMap;
    }
    else if constexpr (std::same_as<T, EditorWallSpriteSet>) {
        return wallMap;
    }
    else if constexpr (std::same_as<T, EditorObjectSpriteSet>) {
        return objectMap;
    }
    else {
        LOG_FATAL("Invalid sprite set type.");
        return floorMap;
    }
}

template<typename T>
bool SpriteDataModel::spriteSetNameIsUnique(Uint16 spriteSetID,
                                            const std::string& displayName)
{
    auto& spriteSetMap{getMapForSpriteSetType<T>()};

    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& spriteSetPair : spriteSetMap) {
        int idToCheck{spriteSetPair.first};
        const T& spriteSet{spriteSetPair.second};

        if ((idToCheck != spriteSetID) && (displayName == spriteSet.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

template<typename T>
void SpriteDataModel::setSpriteSetDisplayName(SpriteSet::Type type,
                                              Uint16 spriteSetID,
                                              const std::string& newDisplayName)
{
    auto& spriteSetMap{getMapForSpriteSetType<T>()};

    auto spriteSetPair{spriteSetMap.find(spriteSetID)};
    if (spriteSetPair == spriteSetMap.end()) {
        LOG_FATAL("Tried to set name using invalid sprite set ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!spriteSetNameIsUnique<T>(spriteSetID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    T& spriteSet{spriteSetPair->second};
    spriteSet.displayName = uniqueDisplayName;

    // Signal the change.
    spriteSetDisplayNameChangedSig.publish(type, spriteSetID,
                                           spriteSet.displayName);
}

template<typename T>
void SpriteDataModel::setSpriteSetSlot(SpriteSet::Type type, Uint16 spriteSetID,
                                       std::size_t index, int newSpriteID)
{
    auto& spriteSetMap{getMapForSpriteSetType<T>()};

    auto spriteSetPair{spriteSetMap.find(spriteSetID)};
    if (spriteSetPair == spriteSetMap.end()) {
        LOG_FATAL("Tried to set slot using invalid sprite set ID.");
    }

    // Check that the specified set can hold the given index.
    T& spriteSet{spriteSetPair->second};
    if (index >= spriteSet.spriteIDs.size()) {
        LOG_FATAL("Tried to set slot using out of bounds index.");
    }

    spriteSet.spriteIDs[index] = newSpriteID;

    // Signal the change.
    spriteSetSlotChangedSig.publish(type, spriteSetID, index, newSpriteID);
}

void SpriteDataModel::saveSpriteSheets(nlohmann::json& json)
{
    json["spriteSheets"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
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
                = deriveStringID(sprite.displayName);

            // Add the numeric ID.
            json["spriteSheets"][i]["sprites"][j]["numericID"]
                = sprite.numericID;

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
}

void SpriteDataModel::saveFloors(nlohmann::json& json)
{
    json["floors"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : floorMap) {
        // Add this sheet's relative path.
        EditorFloorSpriteSet& spriteSet{spriteSetPair.second};
        json["floors"][i]["displayName"] = spriteSet.displayName;
        json["floors"][i]["stringID"] = deriveStringID(spriteSet.displayName);
        json["floors"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["floors"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

void SpriteDataModel::saveFloorCoverings(nlohmann::json& json)
{
    json["floorCoverings"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : floorCoveringMap) {
        // Add this sheet's relative path.
        EditorFloorCoveringSpriteSet& spriteSet{spriteSetPair.second};
        json["floorCoverings"][i]["displayName"] = spriteSet.displayName;
        json["floorCoverings"][i]["stringID"] = deriveStringID(spriteSet.displayName);
        json["floorCoverings"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["floorCoverings"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

void SpriteDataModel::saveWalls(nlohmann::json& json)
{
    json["walls"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : wallMap) {
        // Add this sheet's relative path.
        EditorWallSpriteSet& spriteSet{spriteSetPair.second};
        json["walls"][i]["displayName"] = spriteSet.displayName;
        json["walls"][i]["stringID"] = deriveStringID(spriteSet.displayName);
        json["walls"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["walls"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

void SpriteDataModel::saveObjects(nlohmann::json& json)
{
    json["objects"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : objectMap) {
        // Add this sheet's relative path.
        EditorObjectSpriteSet& spriteSet{spriteSetPair.second};
        json["objects"][i]["displayName"] = spriteSet.displayName;
        json["objects"][i]["stringID"] = deriveStringID(spriteSet.displayName);
        json["objects"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["objects"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

void SpriteDataModel::removeSpriteIDFromSets(int spriteID)
{
    // Given one of the sprite set maps, this will replace all instances of 
    // oldSpriteID with newSpriteID and emit a spriteSetSpriteIDChanged.
    auto replaceIDInSets
        = [this]<typename T>(std::map<Uint16, T>& spriteSetMap,
                             SpriteSet::Type type, int spriteID) { 
        // For each sprite set in the map.
        for (auto& pair : spriteSetMap) {
            T& spriteSet{pair.second};

            // For each slot in the set.
            for (std::size_t i = 0; i < spriteSet.spriteIDs.size(); ++i) {
                // If the ID matches oldSpriteID, replace it and signal.
                int& slotSpriteID{spriteSet.spriteIDs[i]};
                if (slotSpriteID == spriteID) {
                    slotSpriteID = EMPTY_SPRITE_ID;
                    spriteSetSlotChangedSig.publish(
                        type, spriteSet.numericID, static_cast<Uint16>(i),
                        EMPTY_SPRITE_ID);
                }
            }
        }
    };

    replaceIDInSets(floorMap, SpriteSet::Type::Floor, spriteID);
    replaceIDInSets(floorCoveringMap, SpriteSet::Type::FloorCovering, spriteID);
    replaceIDInSets(wallMap, SpriteSet::Type::Wall, spriteID);
    replaceIDInSets(objectMap, SpriteSet::Type::Object,
                    spriteID);
}

void SpriteDataModel::resetModelState()
{
    workingFilePath = "";
    workingTexturesDir = "";
    spriteSheetMap.clear();
    spriteMap.clear();
    floorMap.clear();
    floorCoveringMap.clear();
    wallMap.clear();
    objectMap.clear();
    sheetIDPool.freeAllIDs();
    spriteIDPool.freeAllIDs();
    floorIDPool.freeAllIDs();
    floorCoveringIDPool.freeAllIDs();
    wallIDPool.freeAllIDs();
    objectIDPool.freeAllIDs();
}

} // End namespace SpriteEditor
} // End namespace AM
