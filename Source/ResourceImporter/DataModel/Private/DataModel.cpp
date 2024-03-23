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
namespace ResourceImporter
{
DataModel::DataModel(SDL_Renderer* inSdlRenderer)
: boundingBoxModel{*this}
, graphicSetModel{*this}
, entityGraphicSetModel{*this}
, spriteModel{*this, graphicSetModel, entityGraphicSetModel, inSdlRenderer}
, animationModel{*this}
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
        if (!boundingBoxModel.load(json)) {
            parseError = boundingBoxModel.getErrorString();
        }
        else if (!spriteModel.load(json)) {
            parseError = spriteModel.getErrorString();
        }
        else if (!animationModel.load(json)) {
            parseError = animationModel.getErrorString();
        }
        else if (!graphicSetModel.load(json)) {
            parseError = graphicSetModel.getErrorString();
        }
        else if (!entityGraphicSetModel.load(json)) {
            parseError = entityGraphicSetModel.getErrorString();
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
    boundingBoxModel.save(json);
    spriteModel.save(json);
    animationModel.save(json);
    graphicSetModel.save(json);
    entityGraphicSetModel.save(json);
    iconModel.save(json);

    // Write the json to our working file.
    std::ofstream workingFile(workingFilePath, std::ios::trunc);
    if (!(workingFile.is_open())) {
        LOG_FATAL("File failed to open: %s.", workingFilePath.c_str());
    }

    std::string jsonDump{json.dump(4)};
    workingFile << jsonDump;
}

EditorGraphicRef DataModel::getGraphic(GraphicID graphicID)
{
    if (isSpriteID(graphicID)) {
        return EditorGraphicRef{spriteModel.getSprite(toSpriteID(graphicID))};
    }
    else {
        return EditorGraphicRef{
            animationModel.getAnimation(toAnimationID(graphicID))};
    }
}

void DataModel::setActiveBoundingBox(BoundingBoxID newActiveBoundingBoxID)
{
    // Note: This will error if the ID is invalid. This is good, since we don't
    //       expect any invalid IDs to be floating around.
    const EditorBoundingBox& boundingBox{
        boundingBoxModel.getBoundingBox(newActiveBoundingBoxID)};

    // Signal the active bounding box to the UI.
    activeLibraryItemChangedSig.publish(boundingBox);
}

void DataModel::setActiveSprite(SpriteID newActiveSpriteID)
{
    // Note: This will error if the sprite ID is invalid. This is good, since
    //       we don't expect any invalid IDs to be floating around.
    const EditorSprite& sprite{spriteModel.getSprite(newActiveSpriteID)};

    // Signal the active sprite to the UI.
    activeLibraryItemChangedSig.publish(sprite);
}

void DataModel::setActiveAnimation(AnimationID newActiveAnimationID)
{
    // Note: This will error if the animation ID is invalid. This is good, since
    //       we don't expect any invalid IDs to be floating around.
    const EditorAnimation& animation{
        animationModel.getAnimation(newActiveAnimationID)};

    // Signal the active animation to the UI.
    activeLibraryItemChangedSig.publish(animation);
}

void DataModel::setActiveGraphicSet(GraphicSet::Type type,
                                    Uint16 newActiveGraphicSetID)
{
    // Match the sprite set type, then:
    //   1. Get the set with the given ID (error if it doesn't exist).
    //   2. Signal it out.
    switch (type) {
        case GraphicSet::Type::Floor: {
            activeLibraryItemChangedSig.publish(
                graphicSetModel.getFloor(newActiveGraphicSetID));
            return;
        }
        case GraphicSet::Type::FloorCovering: {
            activeLibraryItemChangedSig.publish(
                graphicSetModel.getFloorCovering(newActiveGraphicSetID));
            return;
        }
        case GraphicSet::Type::Wall: {
            activeLibraryItemChangedSig.publish(
                graphicSetModel.getWall(newActiveGraphicSetID));
            return;
        }
        case GraphicSet::Type::Object: {
            activeLibraryItemChangedSig.publish(
                graphicSetModel.getObject(newActiveGraphicSetID));
            return;
        }
        case GraphicSet::Type::Entity: {
            activeLibraryItemChangedSig.publish(
                entityGraphicSetModel.getEntity(newActiveGraphicSetID));
            return;
        }
        default: {
            LOG_FATAL("Unsupported graphic set type: %u", type);
        }
    }
}

void DataModel::setActiveIcon(IconID newActiveIconID)
{
    // Note: This will error if the ID is invalid. This is good, since we don't
    //       expect any invalid IDs to be floating around.
    const EditorIcon& icon{iconModel.getIcon(newActiveIconID)};

    // Signal the active icon to the UI.
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
    graphicSetModel.resetModelState();
    entityGraphicSetModel.resetModelState();
    iconModel.resetModelState();
}

} // End namespace ResourceImporter
} // End namespace AM
