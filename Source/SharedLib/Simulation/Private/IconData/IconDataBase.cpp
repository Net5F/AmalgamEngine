#include "IconDataBase.h"
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
IconDataBase::IconDataBase(const nlohmann::json& resourceDataJson)
: icons{}
, iconStringMap{}
{
    // Parse the json structure to construct our icons.
    parseJson(resourceDataJson);
}

const Icon& IconDataBase::getIcon(std::string_view stringID) const
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    // Attempt to find the given string ID.
    auto it{iconStringMap.find(workStringID)};
    if (it == iconStringMap.end()) {
        LOG_ERROR("Failed to find icon with string ID: %s",
                  workStringID.c_str());
        return icons[0];
    }

    return *(it->second);
}

const Icon& IconDataBase::getIcon(IconID numericID) const
{
    if (numericID >= icons.size()) {
        LOG_ERROR("Invalid numeric ID while getting icon: %d", numericID);
        return icons[0];
    }

    return icons[numericID];
}

const std::vector<Icon>& IconDataBase::getAllIcons() const
{
    return icons;
}

void IconDataBase::parseJson(const nlohmann::json& json)
{
    // Add the null icon.
    icons.emplace_back("Null", "null", NULL_ICON_ID);

    // Parse the json and catch any parsing errors.
    try {
        // Iterate every icon sheet and add all of their icons.
        for (auto& sheetJson : json.at("iconSheets").items()) {
            for (auto& iconJson : sheetJson.value().at("icons").items()) {
                // Parse the icon's data and add it to our containers.
                parseIcon(iconJson.value());
            }
        }
    } catch (nlohmann::json::type_error& e) {
        LOG_FATAL("Failed to parse icon sheets in ResourceData.json: %s",
                  e.what());
    }

    // Add everything to the associated maps.
    for (const Icon& icon : icons) {
        iconStringMap.emplace(icon.stringID, &icons[icon.numericID]);
    }
}

void IconDataBase::parseIcon(const nlohmann::json& iconJson)
{
    // Add the icon to the icons vector.
    Icon& icon{icons.emplace_back()};

    // Add the display name and IDs.
    icon.numericID = iconJson.at("numericID");
    icon.displayName = iconJson.at("displayName").get<std::string>();
    icon.stringID = iconJson.at("stringID").get<std::string>();
}

} // End namespace AM
