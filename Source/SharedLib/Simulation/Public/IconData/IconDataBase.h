#pragma once

#include "Icon.h"
#include "nlohmann/json_fwd.hpp"
#include <vector>
#include <unordered_map>
#include <string>

namespace AM
{

/**
 * Base class for Client::IconData and Server::IconData.
 * Loads the shared icon data from ResourceData.json into memory and provides
 * an interface for accessing it.
 *
 * Also adds the "Null" icon, for use as a default.
 *
 * The data in this class is immutable. To modify it, edit ResourceData.json
 * using the ResourceImporter.
 */
class IconDataBase
{
public:
    /**
     * Attempts to parse resourceDataJson and load all of the icon data.
     *
     * Errors if resourceDataJson doesn't contain an iconSheets section.
     */
    IconDataBase(const nlohmann::json& resourceDataJson);

    /**
     * Returns the icon with the given string ID.
     * Errors if the given ID doesn't exist.
     */
    const Icon& getIcon(const std::string& stringID);

    /**
     * Returns the icon with the given numeric ID.
     * Errors if the given ID doesn't exist.
     */
    const Icon& getIcon(IconID numericID) const;

    /**
     * Returns a reference to a vector containing all the icons.
     */
    const std::vector<Icon>& getAllIcons() const;

private:
    /**
     * Parses the given json, constructing icons and pushing them into the
     * icons vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              ResourceData.json.
     */
    void parseJson(const nlohmann::json& json);

    /**
     * Parses the given icon json and adds the resulting icon to the
     * icons vector.

     * @param iconJson  The json to parse. Must be a valid icon section
     *                    from ResourceData.json.
     */
    void parseIcon(const nlohmann::json& iconJson);

    /** The loaded icons, indexed by their numeric IDs. */
    std::vector<Icon> icons;

    /** A map for easily looking up icons by their string ID. */
    std::unordered_map<std::string, const Icon*> iconStringMap;

    /** A scratch buffer used while processing string IDs. */
    std::string workStringID;
};

} // End namespace AM
