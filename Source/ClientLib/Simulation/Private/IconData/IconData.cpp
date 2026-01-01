#include "IconData.h"
#include "StringTools.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"

namespace
{
/** A scratch buffer used while processing string IDs.
    Must be file-local so it can be accessed by const functions. */
std::string workStringID{};
}

namespace AM
{
namespace Client
{
IconData::IconData(const nlohmann::json& resourceDataJson)
: IconDataBase(resourceDataJson)
{
    // Parse the json structure to construct our icon render data.
    parseJson(resourceDataJson);
}

const IconRenderData& IconData::getRenderData(std::string_view stringID) const
{
    const Icon& icon{getIcon(stringID)};
    return getRenderData(icon.numericID);
}

const IconRenderData& IconData::getRenderData(IconID numericID) const
{
    if (numericID >= renderData.size()) {
        LOG_FATAL("Invalid numeric ID while getting icon render data: %d",
                  numericID);
    }

    return renderData[numericID];
}

void IconData::parseJson(const nlohmann::json& json)
{
    // Parse the json and catch any parsing errors.
    try {
        // Add the null icon, giving it our engine-default texture.
        renderData.emplace_back(Paths::TEXTURE_DIR + "Defaults/Icon.png",
                                SDL_Rect{0, 0, 64, 64});

        // Parse every icon sheet in the json.
        for (auto& sheetJson : json["iconSheets"].items()) {
            // Get this sheet's texture path.
            std::string texturePath{Paths::TEXTURE_DIR};
            texturePath += sheetJson.value()["relPath"].get<std::string>();

            // Parse every icon in this sheet.
            for (auto& iconJson : sheetJson.value()["icons"].items()) {
                parseIcon(iconJson.value(), texturePath);
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL("Failed to parse icon sheets in ResourceData.json: %s",
                  e.what());
    }
}

void IconData::parseIcon(const nlohmann::json& iconJson,
                         const std::string& iconSheetRelPath)
{
    // Add the render data to the renderData vector.
    IconRenderData& iconRenderData{renderData.emplace_back()};

    // Add the parent icon sheet's path and texture.
    iconRenderData.iconSheetRelPath = iconSheetRelPath;

    // Add this icon's extent within the icon sheet.
    iconRenderData.textureExtent.x = iconJson.at("textureExtent").at("x");
    iconRenderData.textureExtent.y = iconJson.at("textureExtent").at("y");
    iconRenderData.textureExtent.w = iconJson.at("textureExtent").at("w");
    iconRenderData.textureExtent.h = iconJson.at("textureExtent").at("h");
}

} // End namespace Client
} // End namespace AM
