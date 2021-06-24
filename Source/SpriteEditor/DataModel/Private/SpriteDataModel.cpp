#include "SpriteDataModel.h"
#include "Log.h"
#include "nlohmann/json.hpp"

namespace AM
{
namespace SpriteEditor
{

bool SpriteDataModel::create(std::filesystem::path filePath)
{
    // If a SpriteData.json already exists at the given path, return false.
    filePath /= "SpriteData.json";
    std::ifstream existingFile{filePath};
    if (existingFile) {
        return false;
    }

    // Create the file.
    currentWorkingFile.open(filePath, std::ios::out);
    currentWorkingFile.close();

    // Open the file for read and write.
    currentWorkingFile.open(filePath, (std::ios::in | std::ios::out));
    if (!(currentWorkingFile.is_open())) {
        // File creation failed for some reason. We're already checking for
        // file existence so this shouldn't happen.
        LOG_ERROR("Failed to open file.");
    }

    // Save our empty model structure.
    save();

    return true;
}

std::string SpriteDataModel::parse(std::filesystem::path filePath)
{
    // Open the file.
    currentWorkingFile.open(filePath, (std::ios::in | std::ios::out));
    if (!(currentWorkingFile.is_open())) {
        return "File failed to open.";
    }

    // Parse the file into a json structure.
    nlohmann::json json = nlohmann::json::parse(currentWorkingFile, nullptr, false);
    if (json.is_discarded()) {
        return "File is not valid JSON.";
    }

    // Parse the json structure to fill our data model.
    try {
        // For every sprite sheet in the json.
        for (auto& sheetJson : json["spriteSheets"].items()) {
            // Add this sheet's relative path.
            SpriteSheet spriteSheet{};
            spriteSheet.path = sheetJson.value()["path"].get<std::string>();

            // For every sprite in the sheet.
            for (auto& spriteJson : sheetJson.value()["sprites"].items()) {
                // Add this sprite's key.
                SpriteStaticData sprite{};
                std::string key{spriteJson.value()["key"].get<std::string>()};
                sprite.key = entt::hashed_string{key.c_str()};

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

                spriteSheet.sprites.push_back(sprite);
            }

            spriteSheets.push_back(spriteSheet);
        }
    }
    catch (nlohmann::json::type_error& e) {
        std::string failureString{"Parse failure - "};
        failureString += e.what();
        return failureString;
    }

    return "";
}

void SpriteDataModel::save()
{
    if (!(currentWorkingFile.is_open())) {
        // Somehow got here with no file open.
        LOG_ERROR("Tried to save while file wasn't open.");
    }

    // Create a new json structure to fill.
    nlohmann::json json;
    json["spriteSheets"] = nlohmann::json::array();

    // For each sprite sheet.
    for (unsigned int i = 0; i < spriteSheets.size(); ++i) {
        // Add this sheet's relative path.
        SpriteSheet& spriteSheet{spriteSheets[i]};
        json["spriteSheets"][i]["path"] = spriteSheet.path.string();

        // For each sprite in this sheet.
        for (unsigned int j = 0; j < spriteSheet.sprites.size(); ++j) {
            // Add this sprite's key.
            SpriteStaticData& sprite{spriteSheet.sprites[j]};
            json["spriteSheets"][i]["sprites"][j]["key"] = sprite.key.data();

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

    std::string jsonDump{json.dump(4)};
    currentWorkingFile.write(jsonDump.c_str(), jsonDump.length());
}

} // End namespace SpriteEditor
} // End namespace AM
