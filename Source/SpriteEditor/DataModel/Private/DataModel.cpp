#include "DataModel.h"
#include "Log.h"
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
DataModel::DataModel(SDL_Renderer* inSdlRenderer)
: spriteSetModel{*this}
, spriteModel{*this, spriteSetModel, inSdlRenderer}
, iconModel{*this, inSdlRenderer}
, workingFilePath{""}
, workingTexturesDir{""}
, activeLibraryItemChangedSig{}
, activeLibraryItemChanged{activeLibraryItemChangedSig}
{
}

bool DataModel::create(const std::string& fullPath)
{
    // If a ResourceData.json already exists at the given path, return false.
    workingFilePath = fullPath;
    workingFilePath += "/ResourceData.json";
    if (std::filesystem::exists(workingFilePath)) {
        workingFilePath = "";
        errorString
            = "ResourceData.json file already exists at the selected path.";
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

bool DataModel::load(const std::string& fullPath)
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

    // Parse the file into a json structure.
    nlohmann::json json;
    std::string parseError{""};
    try {
        json = nlohmann::json::parse(workingFile, nullptr);
    } catch (nlohmann::json::exception& e) {
        parseError = e.what();
    }

    // Load the data into each model.
    if (parseError == "") {
        if (!spriteModel.load(json)) {
            parseError = spriteModel.getErrorString();
        }
        else if (!spriteSetModel.load(json)) {
            parseError = spriteSetModel.getErrorString();
        }
        else if (!iconModel.load(json)) {
            parseError = iconModel.getErrorString();
        }
    }

    if (parseError != "") {
        resetModelState();
        errorString = "Parse failure - " + parseError;
        return false;
    }

    return true;
}

void DataModel::save()
{
    // Save each part of the model to a json object.
    nlohmann::json json;
    spriteModel.save(json);
    spriteSetModel.save(json);
    iconModel.save(json);

    // Write the json to our working file.
    std::ofstream workingFile(workingFilePath, std::ios::trunc);
    if (!(workingFile.is_open())) {
        LOG_FATAL("File failed to open: %s.", workingFilePath.c_str());
    }

    std::string jsonDump{json.dump(4)};
    workingFile << jsonDump;
}

void DataModel::setActiveSprite(int newActiveSpriteID)
{
    // Note: This will error if the sprite ID is invalid. This is good, since 
    //       we don't expect any invalid IDs to be floating around.
    const EditorSprite& sprite{spriteModel.getSprite(newActiveSpriteID)};

    // Signal the active sprite to the UI.
    activeLibraryItemChangedSig.publish(sprite);
}

void DataModel::setActiveSpriteSet(SpriteSet::Type type, Uint16 newActiveSpriteSetID)
{
    // Match the sprite set type, then:
    //   1. Get the set with the given ID (error if it doesn't exist).
    //   2. Signal it out.
    switch (type) {
        case SpriteSet::Type::Floor: {
            activeLibraryItemChangedSig.publish(
                spriteSetModel.getFloor(newActiveSpriteSetID));
            return;
        }
        case SpriteSet::Type::FloorCovering: {
            activeLibraryItemChangedSig.publish(
                spriteSetModel.getFloorCovering(newActiveSpriteSetID));
            return;
        }
        case SpriteSet::Type::Wall: {
            activeLibraryItemChangedSig.publish(
                spriteSetModel.getWall(newActiveSpriteSetID));
            return;
        }
        case SpriteSet::Type::Object: {
            activeLibraryItemChangedSig.publish(
                spriteSetModel.getObject(newActiveSpriteSetID));
            return;
        }
        default: {
            LOG_FATAL("Unsupported sprite set type: %u", type);
        }
    }
}

void DataModel::setActiveIcon(IconID newActiveIconID)
{
    // Note: This will error if the sprite ID is invalid. This is good, since 
    //       we don't expect any invalid IDs to be floating around.
    const EditorIcon& icon{iconModel.getIcon(newActiveIconID)};

    // Signal the active sprite to the UI.
    activeLibraryItemChangedSig.publish(icon);
}

const std::string& DataModel::getWorkingTexturesDir()
{
    return workingTexturesDir;
}

const std::string& DataModel::getErrorString()
{
    return errorString;
}

std::string DataModel::deriveStringID(const std::string& displayName)
{
    // Make the string all lowercase.
    std::string stringID{displayName};
    std::transform(stringID.begin(), stringID.end(), stringID.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Replace spaces with underscores.
    std::replace(stringID.begin(), stringID.end(), ' ', '_');

    return stringID;
}

bool DataModel::validateRelPath(const std::string& relPath)
{
    // Construct the file path.
    std::filesystem::path filePath{getWorkingTexturesDir()};
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

bool DataModel::setWorkingTexturesDir()
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

void DataModel::resetModelState()
{
    workingFilePath = "";
    workingTexturesDir = "";

    spriteModel.resetModelState();
    spriteSetModel.resetModelState();
    iconModel.resetModelState();
}

} // End namespace SpriteEditor
} // End namespace AM
