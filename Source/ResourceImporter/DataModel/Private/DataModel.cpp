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
: spriteModel{*this, inSdlRenderer}
, animationModel{*this}
, boundingBoxModel{*this}
, graphicSetModel{*this}
, entityGraphicSetModel{*this}
, iconModel{*this, inSdlRenderer}
, workingJsonPath{""}
, workingTexturesDir{""}
, workingIndividualSpritesDir{""}
, activeLibraryItemChangedSig{}
, activeLibraryItemChanged{activeLibraryItemChangedSig}
{
}

bool DataModel::open(std::string_view resourcesPath)
{
    // Check that the selected path is a directory named "Resources".
    if (!(resourcesPath.ends_with("Resources"))) {
        errorString
            = "Please select the \"Resources\" directory in your project.";
        return false;
    }

    // If there's already a ResourceData.json, load it.
    workingJsonPath = resourcesPath;
    workingJsonPath += "/Shared/Common/ResourceData.json";
    bool result{false};
    if (std::filesystem::exists(workingJsonPath)) {
        result = loadJson(workingJsonPath);
    }
    else {
        // Json doesn't exist. Create it.
        std::string jsonParentPath{resourcesPath};
        jsonParentPath += "/Shared/Common/";
        result = createJson(jsonParentPath);
    }

    // If the operation failed, return early.
    if (!result) {
        workingJsonPath = "";
        return false;
    }

    // Check that the asset directories exist.
    std::string texturesDir{resourcesPath};
    texturesDir.append("/Client/Common/Assets/Textures");
    if (std::filesystem::exists(texturesDir)) {
        workingTexturesDir = texturesDir;
    }
    else {
        errorString = "Textures directory does not exist. Please adopt the "
                      "expected project layout.";
        return false;
    }

    std::string individualSpritessDir{resourcesPath};
    individualSpritessDir.append("/Client/Common/Assets/IndividualSprites");
    if (std::filesystem::exists(individualSpritessDir)) {
        workingIndividualSpritesDir = individualSpritessDir;
    }
    else {
        errorString = "IndividualSprites directory does not exist. Please "
                      "adopt the expected project layout.";
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
    std::ofstream workingFile(workingJsonPath, std::ios::trunc);
    if (!(workingFile.is_open())) {
        LOG_FATAL("File failed to open: %s.", workingJsonPath.c_str());
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

void DataModel::setActiveSpriteSheet(int newActiveSpriteSheetID)
{
    // Note: This will error if the sheet ID is invalid. This is good, since
    //       we don't expect any invalid IDs to be floating around.
    const EditorSpriteSheet& spriteSheet{
        spriteModel.getSpriteSheet(newActiveSpriteSheetID)};

    // Signal the active sprite to the UI.
    activeLibraryItemChangedSig.publish(spriteSheet);
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

void DataModel::setActiveBoundingBox(BoundingBoxID newActiveBoundingBoxID)
{
    // Note: This will error if the ID is invalid. This is good, since we don't
    //       expect any invalid IDs to be floating around.
    const EditorBoundingBox& boundingBox{
        boundingBoxModel.getBoundingBox(newActiveBoundingBoxID)};

    // Signal the active bounding box to the UI.
    activeLibraryItemChangedSig.publish(boundingBox);
}

void DataModel::setActiveGraphicSet(GraphicSet::Type type,
                                    Uint16 newActiveGraphicSetID)
{
    // Match the sprite set type, then:
    //   1. Get the set with the given ID (error if it doesn't exist).
    //   2. Signal it out.
    switch (type) {
        case GraphicSet::Type::Terrain: {
            activeLibraryItemChangedSig.publish(
                graphicSetModel.getTerrain(newActiveGraphicSetID));
            return;
        }
        case GraphicSet::Type::Floor: {
            activeLibraryItemChangedSig.publish(
                graphicSetModel.getFloor(newActiveGraphicSetID));
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

const std::string& DataModel::getWorkingIndividualSpritesDir()
{
    return workingIndividualSpritesDir;
}

const std::string& DataModel::getErrorString()
{
    return errorString;
}

bool DataModel::validateRelPath(const std::string& relPath)
{
    // Construct the file path.
    std::filesystem::path filePath{getWorkingTexturesDir()};
    filePath.append(relPath);

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

bool DataModel::createJson(const std::string& inJsonFilePath)
{
    // Create the file.
    std::ofstream workingJson(inJsonFilePath, std::ios::app);
    workingJson.close();

    // Save our empty model structure.
    save();

    return true;
}

bool DataModel::loadJson(const std::string& inJsonFilePath)
{
    // Open the file.
    std::ifstream workingJsonFile(inJsonFilePath);
    if (!(workingJsonFile.is_open())) {
        errorString = "File failed to open.";
        return false;
    }

    // Parse the file into a json structure.
    nlohmann::json json;
    std::string parseError{""};
    try {
        json = nlohmann::json::parse(workingJsonFile, nullptr);
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
        errorString = parseError;
        return false;
    }

    return true;
}

void DataModel::resetModelState()
{
    workingJsonPath = "";
    workingTexturesDir = "";
    workingIndividualSpritesDir = "";

    spriteModel.resetModelState();
    graphicSetModel.resetModelState();
    entityGraphicSetModel.resetModelState();
    iconModel.resetModelState();
}

} // End namespace ResourceImporter
} // End namespace AM
