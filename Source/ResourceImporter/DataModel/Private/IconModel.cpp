#include "IconModel.h"
#include "DataModel.h"
#include "nlohmann/json.hpp"
#include <SDL_render.h>
#include <SDL_image.h>
#include <filesystem>
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{

IconModel::IconModel(DataModel& inDataModel, SDL_Renderer* inSdlRenderer)
: dataModel{inDataModel}
, sdlRenderer{inSdlRenderer}
, iconSheetMap{}
, iconMap{}
, sheetIDPool{MAX_ICON_SHEETS}
, iconIDPool{MAX_ICONS}
, errorString{}
, sheetAddedSig{}
, sheetRemovedSig{}
, iconRemovedSig{}
, iconDisplayNameChangedSig{}
, sheetAdded{sheetAddedSig}
, sheetRemoved{sheetRemovedSig}
, iconRemoved{iconRemovedSig}
, iconDisplayNameChanged{iconDisplayNameChangedSig}
{
    // Reserve the null icon's ID (the engine provides it in code, so we don't 
    // need it in the json).
    iconIDPool.reserveID(NULL_ICON_ID);
}

bool IconModel::load(const nlohmann::json& json)
{
    try {
        // Iterate every icon sheet and add all their icons.
        for (auto& sheetJson : json.at("iconSheets").items()) {
            if (!parseIconSheet(sheetJson.value())) {
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

void IconModel::save(nlohmann::json& json)
{
    json["iconSheets"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each icon sheet.
    int i{0};
    for (auto& sheetPair : iconSheetMap) {
        // Add this sheet's relative path.
        EditorIconSheet& iconSheet{sheetPair.second};
        json["iconSheets"][i]["relPath"] = iconSheet.relPath;

        // For each icon in this sheet.
        for (std::size_t j = 0; j < iconSheet.iconIDs.size(); ++j) {
            // Add the display name.
            EditorIcon& icon{iconMap[iconSheet.iconIDs[j]]};
            json["iconSheets"][i]["icons"][j]["displayName"]
                = icon.displayName;

            // Derive the string ID from the display name and add it.
            json["iconSheets"][i]["icons"][j]["stringID"]
                = DataModel::deriveStringID(icon.displayName);

            // Add the numeric ID.
            json["iconSheets"][i]["icons"][j]["numericID"]
                = icon.numericID;

            // Add the icon sheet texture extent.
            json["iconSheets"][i]["icons"][j]["textureExtent"]["x"]
                = icon.textureExtent.x;
            json["iconSheets"][i]["icons"][j]["textureExtent"]["y"]
                = icon.textureExtent.y;
            json["iconSheets"][i]["icons"][j]["textureExtent"]["w"]
                = icon.textureExtent.w;
            json["iconSheets"][i]["icons"][j]["textureExtent"]["h"]
                = icon.textureExtent.h;
        }

        i++;
    }
}

bool IconModel::addIconSheet(const std::string& relPath,
                             const std::string& iconWidth,
                             const std::string& iconHeight,
                             const std::string& baseName)
{
    /* Validate the data. */
    // Check if we already have the given sheet.
    for (const auto& sheetPair : iconSheetMap) {
        if (sheetPair.second.relPath == relPath) {
            errorString = "Error: Path conflicts with existing icon sheet.";
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
    int iconWidthI{0};
    int iconHeightI{0};
    try {
        iconWidthI = std::stoi(iconWidth);
        iconHeightI = std::stoi(iconHeight);
    } catch (std::exception&) {
        errorString
            = "Error: Width or height is not a valid integer.";
        return false;
    }

    // Validate the size of the texture.
    if ((iconWidthI > sheetWidth) || (iconHeightI > sheetHeight)) {
        errorString = "Error: Sheet must be larger than icon size.";
        return false;
    }

    /* Add the icon sheet and icons. */
    int sheetID{static_cast<int>(sheetIDPool.reserveID())};
    iconSheetMap.emplace(sheetID, EditorIconSheet{relPath, {}});

    // For each icon in this texture.
    EditorIconSheet& iconSheet{iconSheetMap[sheetID]};
    int iconCount{0};
    for (int y = 0; y <= (sheetHeight - iconHeightI); y += iconHeightI) {
        for (int x = 0; x <= (sheetWidth - iconWidthI); x += iconWidthI) {
            // Build the icon's display name (baseName_count).
            std::string displayName{baseName};
            displayName += std::to_string(iconCount);

            // Find the icon's extent within the sheet texture.
            SDL_Rect textureExtent{x, y, iconWidthI, iconHeightI};

            // Add the icon to the map and sheet.
            IconID iconID{static_cast<IconID>(iconIDPool.reserveID())};
            iconMap.emplace(iconID, EditorIcon{iconID, iconSheet.relPath,
                                               displayName, textureExtent});
            iconSheet.iconIDs.push_back(iconID);

            // Increment the count (used for the display name).
            iconCount++;
        }
    }

    // Signal the new sheet to the UI.
    sheetAddedSig.publish(sheetID, iconSheet);

    return true;
}

void IconModel::remIconSheet(int sheetID)
{
    // Find the sheet in the map.
    auto sheetIt{iconSheetMap.find(sheetID)};
    if (sheetIt == iconSheetMap.end()) {
        LOG_FATAL("Invalid ID while removing icon sheet.");
    }

    // Erase all of the sheet's icons.
    for (IconID iconID : sheetIt->second.iconIDs) {
        remIcon(iconID);
    }

    // Erase the sheet.
    iconSheetMap.erase(sheetIt);

    // Signal that the sheet was erased.
    sheetRemovedSig.publish(sheetID);
}

void IconModel::remIcon(IconID iconID)
{
    // Find the icon in the map.
    auto iconIt{iconMap.find(iconID)};
    if (iconIt == iconMap.end()) {
        LOG_FATAL("Invalid ID while removing icon.");
    }

    // Erase the icon.
    iconMap.erase(iconIt);

    // Signal that the icon was erased.
    iconRemovedSig.publish(iconID);
}

const EditorIcon& IconModel::getIcon(IconID iconID)
{
    auto iconIt{iconMap.find(iconID)};
    if (iconIt == iconMap.end()) {
        LOG_FATAL("Tried to get icon with invalid ID: %d", iconID);
    }

    return iconIt->second;
}

void IconModel::setIconDisplayName(IconID iconID,
                                   const std::string& newDisplayName)
{
    auto iconPair{iconMap.find(iconID)};
    if (iconPair == iconMap.end()) {
        LOG_FATAL("Tried to set name using invalid icon ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!iconNameIsUnique(iconID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    EditorIcon& icon{iconPair->second};
    icon.displayName = uniqueDisplayName;

    // Signal the change.
    iconDisplayNameChangedSig.publish(iconID, icon.displayName);
}

void IconModel::resetModelState()
{
    iconSheetMap.clear();
    iconMap.clear();
    sheetIDPool.freeAllIDs();
    iconIDPool.freeAllIDs();
}

const std::string& IconModel::getErrorString()
{
    return errorString;
}

bool IconModel::parseIconSheet(const nlohmann::json& sheetJson)
{
    int sheetID{static_cast<int>(sheetIDPool.reserveID())};
    iconSheetMap.emplace(sheetID, EditorIconSheet{});
    EditorIconSheet& iconSheet{iconSheetMap[sheetID]};

    // Add this sheet's relative path.
    iconSheet.relPath
        = sheetJson.at("relPath").get<std::string>();
    if (!(dataModel.validateRelPath(iconSheet.relPath))) {
        return false;
    }

    // For every icon in the sheet.
    for (auto& iconJson : sheetJson.at("icons").items()) {
        if (!parseIcon(iconJson.value(), iconSheet)) {
            return false;
        }
    }

    // Signal the new sheet to the UI.
    sheetAddedSig.publish(sheetID, iconSheet);

    return true;
}

bool IconModel::parseIcon(const nlohmann::json& iconJson,
                     EditorIconSheet& iconSheet)
{
    // Mark the icon's ID as reserved so it doesn't get reused.
    IconID iconID{iconJson.at("numericID").get<IconID>()};
    iconIDPool.markIDAsReserved(iconID);

    iconMap.emplace(iconID,
                      EditorIcon{iconID, iconSheet.relPath});
    iconSheet.iconIDs.push_back(iconID);
    EditorIcon& icon{iconMap[iconID]};

    // If the display name isn't unique, fail.
    std::string displayName{
        iconJson.at("displayName").get<std::string>()};
    if (!iconNameIsUnique(iconID, displayName)) {
        errorString = "Icon display name isn't unique: ";
        errorString += icon.displayName.c_str();
        return false;
    }

    // Add the display name.
    icon.displayName = displayName;

    // Add this icon's extent within the icon sheet.
    icon.textureExtent.x = iconJson.at("textureExtent").at("x");
    icon.textureExtent.y = iconJson.at("textureExtent").at("y");
    icon.textureExtent.w = iconJson.at("textureExtent").at("w");
    icon.textureExtent.h = iconJson.at("textureExtent").at("h");

    return true;
}

bool IconModel::iconNameIsUnique(IconID iconID, const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& iconPair : iconMap) {
        IconID idToCheck{iconPair.first};
        const EditorIcon& icon{iconPair.second};

        if ((idToCheck != iconID) && (displayName == icon.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

} // End namespace ResourceImporter
} // End namespace AM
