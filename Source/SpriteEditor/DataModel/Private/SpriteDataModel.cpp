#include "SpriteDataModel.h"
#include "Log.h"
#include "Ignore.h"
#include "AUI/Core.h"
#include "nlohmann/json.hpp"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>

#include <filesystem>
#include <algorithm>
#include <cctype>

namespace AM
{
namespace SpriteEditor
{

SpriteDataModel::SpriteDataModel(SDL_Renderer* inSdlRenderer)
: sdlRenderer{inSdlRenderer}
, workingFilePath{""}
, workingResourcesDir{""}
{
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
    if (!setWorkingResourcesDir()) {
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
        if (!setWorkingResourcesDir()) {
            return "Failed to create Resources directory.";
        }
    }
    else {
        return "File failed to open.";
    }

    // Parse the file into a json structure.
    nlohmann::json json = nlohmann::json::parse(workingFile, nullptr, false);
    if (json.is_discarded()) {
        workingFile.close();
        return "File is not valid JSON.";
    }

    // Parse the json structure to fill our data model.
    try {
        // For every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            spriteSheets.push_back(SpriteSheet());
            SpriteSheet& spriteSheet{spriteSheets.back()};

            // Add this sheet's relative path.
            spriteSheet.relPath = sheetJson.value()["relPath"].get<std::string>();
            std::string resultString = validateRelPath(spriteSheet.relPath);
            if (resultString != "") {
                workingFile.close();
                spriteSheets.clear();
                return resultString;
            }

            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                spriteSheet.sprites.push_back(SpriteStaticData{spriteSheet});
                SpriteStaticData& sprite{spriteSheet.sprites.back()};

                // Add this sprite's display name.
                sprite.displayName = spriteJson.value()["displayName"].get<std::string>();

                // If the display name isn't unique, fail.
                if (!displayNameIsUnique(sprite.displayName)) {
                    std::string returnString{"Display name isn't unique: "};
                    returnString += sprite.displayName.c_str();

                    workingFile.close();
                    spriteSheets.clear();
                    return returnString;
                }

                // Add this sprite's sprite sheet texture extent.
                sprite.textureExtent.x = spriteJson.value()["textureExtent"]["x"];
                sprite.textureExtent.y = spriteJson.value()["textureExtent"]["y"];
                sprite.textureExtent.w = spriteJson.value()["textureExtent"]["w"];
                sprite.textureExtent.h = spriteJson.value()["textureExtent"]["h"];

                // Add this sprite's Y offset.
                sprite.yOffset = spriteJson.value()["yOffset"];

                // Add this sprite's model-space bounds.
                sprite.modelBounds.minX = spriteJson.value()["modelBounds"]["minX"];
                sprite.modelBounds.maxX = spriteJson.value()["modelBounds"]["maxX"];
                sprite.modelBounds.minY = spriteJson.value()["modelBounds"]["minY"];
                sprite.modelBounds.maxY = spriteJson.value()["modelBounds"]["maxY"];
                sprite.modelBounds.minZ = spriteJson.value()["modelBounds"]["minZ"];
                sprite.modelBounds.maxZ = spriteJson.value()["modelBounds"]["maxZ"];
            }
        }
    }
    catch (nlohmann::json::type_error& e) {
        workingFile.close();
        spriteSheets.clear();
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
    for (unsigned int i = 0; i < spriteSheets.size(); ++i) {
        // Add this sheet's relative path.
        SpriteSheet& spriteSheet{spriteSheets[i]};
        json["spriteSheets"][i]["relPath"] = spriteSheet.relPath;

        // For each sprite in this sheet.
        for (unsigned int j = 0; j < spriteSheet.sprites.size(); ++j) {
            // Add the display name.
            SpriteStaticData& sprite{spriteSheet.sprites[j]};
            json["spriteSheets"][i]["sprites"][j]["displayName"] = sprite.displayName;

            // Derive the string ID from the display name and add it.
            json["spriteSheets"][i]["sprites"][j]["stringId"] = deriveStringId(sprite.displayName);

            // Add the numeric ID.
            json["spriteSheets"][i]["sprites"][j]["numericId"] = nextNumericId;
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
            json["spriteSheets"][i]["sprites"][j]["yOffset"]
                = sprite.yOffset;

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
    }

    // Write the json to our working file.
    std::ofstream workingFile(workingFilePath, std::ios::trunc);
    if (!(workingFile.is_open())) {
        LOG_ERROR("File failed to open: %s.", workingFilePath.c_str());
    }

    std::string jsonDump{json.dump(4)};
    workingFile << jsonDump;
}

std::string SpriteDataModel::addSpriteSheet(const std::string& relPath, const std::string& spriteWidth
                           , const std::string& spriteHeight, const std::string& yOffset, const std::string& baseName)
{
    /* Validate the data. */
    // Check if we already have the given sheet.
    for (const SpriteSheet& spriteSheet : spriteSheets) {
        if (spriteSheet.relPath == relPath) {
            return "Error: Path conflicts with existing sprite sheet.";
        }
    }

    // Append the texture directory to the given relative path.
    std::string fullPath{workingResourcesDir};
    fullPath += relPath;

    // Validate that the file at the given path is a valid texture.
    int sheetWidth{0};
    int sheetHeight{0};
    SDL_Texture* sheetTexture = IMG_LoadTexture(sdlRenderer, fullPath.c_str());
    if (sheetTexture == nullptr) {
        std::string errorString{"Error: File at given path is not a valid image. Path: "};
        errorString += workingResourcesDir;
        errorString+= relPath;
        return errorString;
    }
    else {
        // Save the texture size for later.
        SDL_QueryTexture(sheetTexture, nullptr, nullptr, &sheetWidth, &sheetHeight);

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
    }
    catch (std::exception& e) {
        return "Error: Width, height, or Y offset is not a valid integer.";
    }

    // Validate the size of the texture.
    if ((spriteWidthI > sheetWidth) || (spriteHeightI > sheetHeight)) {
        return "Error: Sheet must be larger than sprite size.";
    }

    /* Add the sprite sheet and sprites. */
    spriteSheets.emplace_back(relPath);

    // For each sprite in this texture.
    SpriteSheet& spriteSheet{*(spriteSheets.end() - 1)};
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

            // Add the sprite to the sheet.
            spriteSheet.sprites.emplace_back(spriteSheet, displayName
                , textureExtent, yOffsetI, defaultBox);

            // Increment the count (used for the display name).
            spriteCount++;
        }
    }

    return "";
}

void SpriteDataModel::remSpriteSheet(unsigned int index)
{
    if (index >= spriteSheets.size()) {
        LOG_ERROR("Index out of bounds while removing sprite sheet.");
    }

    spriteSheets.erase(spriteSheets.begin() + index);
}

bool SpriteDataModel::displayNameIsUnique(const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a map that we keep updated.
    bool isUnique{true};
    for (const SpriteSheet& sheet : spriteSheets) {
        for (const SpriteStaticData& sprite : sheet.sprites) {
            if (displayName == sprite.displayName) {
                isUnique = false;
            }
        }
    }

    return isUnique;
}

std::vector<SpriteSheet>& SpriteDataModel::getSpriteSheets()
{
    return spriteSheets;
}

const std::string& SpriteDataModel::getWorkingResourcesDir()
{
    return workingResourcesDir;
}

std::string SpriteDataModel::validateRelPath(const std::string& relPath)
{
    // Construct the file path.
    std::filesystem::path filePath{workingResourcesDir};
    filePath /= relPath;

    // Check if the file exists.
    if (std::filesystem::exists(filePath)) {
        return "";
    }
    else {
        // File doesn't exist, return an error string.
        std::string returnString{"File not found at Assets/"};
        returnString += relPath;
        return returnString;
    }
}

bool SpriteDataModel::setWorkingResourcesDir()
{
    // Construct the resources dir path.
    std::filesystem::path resourcesDirPath{workingFilePath};
    resourcesDirPath = resourcesDirPath.parent_path();
    resourcesDirPath /= "Assets/Textures/";

    // Check if the resources dir exists.
    if (!std::filesystem::exists(resourcesDirPath)) {
        // Resources dir doesn't exist, create it.
        std::error_code errorCode;
        if (!std::filesystem::create_directory(resourcesDirPath, errorCode)) {
            // Failed to create dir, return false.
            return false;
        }
    }

    // Convert the path to UTF_8 string and save it.
    const char8_t* u8StringPtr{resourcesDirPath.u8string().c_str()};
    workingResourcesDir = std::string(reinterpret_cast<const char*>(u8StringPtr));

    return true;
}

std::string SpriteDataModel::deriveStringId(const std::string& displayName)
{
    // Make the string all lowercase.
    std::string stringId{displayName};
    std::transform(stringId.begin(), stringId.end(), stringId.begin(),
        [](unsigned char c){ return std::tolower(c); });

    // Replace spaces with underscores.
    std::replace(stringId.begin(), stringId.end(), ' ', '_');

    return stringId;
}

} // End namespace SpriteEditor
} // End namespace AM
