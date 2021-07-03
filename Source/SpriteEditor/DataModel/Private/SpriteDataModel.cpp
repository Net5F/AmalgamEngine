#include "SpriteDataModel.h"
#include "Log.h"
#include "Ignore.h"
#include "AUI/Core.h"
#include "nlohmann/json.hpp"
#include "SDL2pp/Renderer.hh"
#include "SDL2pp/Texture.hh"
#include "SDL2pp/Exception.hh"

namespace AM
{
namespace SpriteEditor
{

SpriteDataModel::SpriteDataModel(SDL2pp::Renderer& inSdlRenderer)
: sdlRenderer{inSdlRenderer}
, currentWorkingFilePath{""}
, nextSpriteId{0}
{
}

bool SpriteDataModel::create(const std::string& fullPath)
{
    // If a SpriteData.json already exists at the given path, return false.
    currentWorkingFilePath = fullPath;
    currentWorkingFilePath += "/SpriteData.json";
    std::ifstream existingFile{currentWorkingFilePath};
    if (existingFile) {
        currentWorkingFilePath = "";
        return false;
    }

    // Create the file.
    std::ofstream currentWorkingFile(currentWorkingFilePath, std::ios::app);
    currentWorkingFile.close();

    // Save our empty model structure.
    save();

    return true;
}

std::string SpriteDataModel::load(const std::string& fullPath)
{
    // Open the file.
    std::ifstream currentWorkingFile(fullPath);
    if (currentWorkingFile.is_open()) {
        currentWorkingFilePath = fullPath;
    }
    else {
        return "File failed to open.";
    }

    // Parse the file into a json structure.
    nlohmann::json json = nlohmann::json::parse(currentWorkingFile, nullptr, false);
    if (json.is_discarded()) {
        currentWorkingFile.close();
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

            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                spriteSheet.sprites.push_back(SpriteStaticData());
                SpriteStaticData& sprite{spriteSheet.sprites.back()};

                // Add this sprite's key.
                sprite.displayName = spriteJson.value()["displayName"].get<std::string>();

                // If the parsed ID is valid.
                int parsedId = spriteJson.value()["id"];
                if (idIsValid(parsedId)) {
                    // Add this sprite's ID.
                    sprite.id = parsedId;

                    // If it's greater than our saved ID, save it.
                    if (sprite.id > nextSpriteId) {
                        nextSpriteId = sprite.id;
                    }
                }
                else {
                    return "Sprite ID is not within range of Uint16.";
                }

                // Add this sprite's sprite sheet texture extent.
                sprite.textureExtent.x = spriteJson.value()["textureExtent"]["x"];
                sprite.textureExtent.y = spriteJson.value()["textureExtent"]["y"];
                sprite.textureExtent.w = spriteJson.value()["textureExtent"]["w"];
                sprite.textureExtent.h = spriteJson.value()["textureExtent"]["h"];

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
        currentWorkingFile.close();
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
    for (unsigned int i = 0; i < spriteSheets.size(); ++i) {
        // Add this sheet's relative path.
        SpriteSheet& spriteSheet{spriteSheets[i]};
        json["spriteSheets"][i]["relPath"] = spriteSheet.relPath;

        // For each sprite in this sheet.
        for (unsigned int j = 0; j < spriteSheet.sprites.size(); ++j) {
            // Add this sprite's display name.
            SpriteStaticData& sprite{spriteSheet.sprites[j]};
            json["spriteSheets"][i]["sprites"][j]["displayName"] = sprite.displayName;

            // Add this sprite's ID.
            json["spriteSheets"][i]["sprites"][j]["id"] = sprite.id;

            // Add this sprite's sprite sheet texture extent.
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["x"]
                = sprite.textureExtent.x;
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["y"]
                = sprite.textureExtent.y;
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["w"]
                = sprite.textureExtent.w;
            json["spriteSheets"][i]["sprites"][j]["textureExtent"]["h"]
                = sprite.textureExtent.h;

            // Add this sprite's model-space bounds.
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
    std::ofstream currentWorkingFile(currentWorkingFilePath, std::ios::trunc);
    if (!(currentWorkingFile.is_open())) {
        LOG_ERROR("File failed to open: %s.", currentWorkingFilePath.c_str());
    }

    std::string jsonDump{json.dump(4)};
    currentWorkingFile << jsonDump;
}

std::string SpriteDataModel::addSpriteSheet(const std::string& relPath, const std::string& spriteWidth
                           , const std::string& spriteHeight, const std::string& baseName)
{
    /* Validate the data. */
    // Check if we already have the given sheet.
    for (const SpriteSheet& spriteSheet : spriteSheets) {
        if (spriteSheet.relPath == relPath) {
            return "Error: Path conflicts with existing sprite sheet.";
        }
    }

    // Validate that the file at the given path is a valid texture.
    int sheetWidth{0};
    int sheetHeight{0};
    try {
        // Append AUI::Core::resourcePath to the given relative path.
        std::string fullPath{AUI::Core::GetResourcePath()};
        fullPath += relPath;
        SDL2pp::Texture sheetTexture(sdlRenderer, fullPath);

        // Save the texture size for later.
        SDL2pp::Point sheetSize = sheetTexture.GetSize();
        sheetWidth = sheetSize.x;
        sheetHeight = sheetSize.y;
    }
    catch (SDL2pp::Exception& e) {
        std::string errorString{"Error: File at given path is not a valid image. Path: "};
        errorString += AUI::Core::GetResourcePath();
        errorString+= relPath;
        return errorString;
    }

    // Validate the width/height.
    int spriteWidthI{0};
    int spriteHeightI{0};
    try {
        spriteWidthI = std::stoi(spriteWidth);
        spriteHeightI = std::stoi(spriteHeight);
    }
    catch (std::exception& e) {
        return "Error: Width or height is not a valid integer.";
    }

    // Validate the size of the texture.
    if ((spriteWidthI > sheetWidth) || (spriteHeightI > sheetHeight)) {
        return "Error: Sheet must be larger than sprite size.";
    }

    /* Add the sprite sheet and sprites. */
    spriteSheets.emplace_back(relPath);

    // For each sprite in this texture.
    SpriteSheet& sheet{*(spriteSheets.end() - 1)};
    int spriteCount{0};
    for (int y = 0; y <= (sheetHeight - spriteHeightI); y += spriteHeightI) {
        for (int x = 0; x <= (sheetWidth - spriteWidthI); x += spriteWidthI) {
            // Build the sprite's display name (baseName_count).
            std::string displayName{baseName};
            displayName += std::to_string(spriteCount);

            // Find the sprite's extent within the sheet texture.
            SDL2pp::Rect textureExtent{x, y, spriteWidthI, spriteHeightI};

            // Default to a non-0 bounding box so it's easier to click.
            static BoundingBox defaultBox{0, 20, 0, 20, 0, 20};

            // Add the sprite to the sheet.
            sheet.sprites.emplace_back(displayName, nextSpriteId
                , textureExtent, defaultBox);

            // Increment the count (used for the display name).
            spriteCount++;

            // Increment the sprite ID and make sure it's still in range.
            if (idIsValid(static_cast<int>(nextSpriteId) + 1)) {
                nextSpriteId++;
            }
            else {
                LOG_ERROR("Sprite ID went out of range of Uint16.");
            }
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

const std::vector<SpriteSheet>& SpriteDataModel::getSpriteSheets()
{
    return spriteSheets;
}

bool SpriteDataModel::idIsValid(int spriteId)
{
    // Check if the ID is in range of a Uint16.
    if ((spriteId >= 0) && (static_cast<unsigned int>(spriteId) <= SDL_MAX_UINT16)) {
        return true;
    }
    else {
        return false;
    }
}

} // End namespace SpriteEditor
} // End namespace AM
