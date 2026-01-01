#pragma once

#include "IconDataBase.h"
#include "IconRenderData.h"
#include <vector>

namespace AM
{
namespace Client
{
/**
 * See IconDataBase class comment.
 *
 * Beyond the parsing that IconDataBase performs, this class loads the
 * rendering-specific icon data and provides an interface for accessing it.
 *
 * The data in this class is immutable. To modify it, edit ResourceData.json
 * using the ResourceImporter.
 */
class IconData : public IconDataBase
{
public:
    /**
     * Calls IconDataBase() constructor, then load the rendering-specific
     * icon data from the given json.
     *
     * Note: We don't need to actually load any textures, since icons are
     *       only used by the UI (which manages its own textures).
     *
     * Errors if resourceDataJson doesn't contain a iconSheets section.
     */
    IconData(const nlohmann::json& resourceDataJson);

    /**
     * Get a icon's render data, using its string ID.
     */
    const IconRenderData& getRenderData(std::string_view stringID) const;

    /**
     * Get a icon's render data, using its numeric ID.
     */
    const IconRenderData& getRenderData(IconID numericID) const;

private:
    /**
     * Parses the given json, constructing icons and pushing them into the
     * renderData vector.
     *
     * @param json  The json to parse. Must be loaded from a valid
     *              ResourceData.json.
     */
    void parseJson(const nlohmann::json& json);

    /**
     * Parses the given icon json and adds the resulting icon to the
     * renderData vector.
     *
     * @param iconJson  The json to parse. Must be a valid icon section
     *                    from ResourceData.json.
     * @param iconSheetRelPath  The relative path to the icon sheet image
     *                            file that holds this icon.
     * @param texture  The texture of the iconsheet that contains the icon.
     */
    void parseIcon(const nlohmann::json& iconJson,
                   const std::string& iconSheetRelPath);

    /** The loaded rendering data for each icon, indexed by their numeric
        IDs */
    std::vector<IconRenderData> renderData;
};

} // End namespace Client
} // End namespace AM
