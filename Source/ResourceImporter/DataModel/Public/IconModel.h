#pragma once

#include "EditorIconSheet.h"
#include "EditorIcon.h"
#include "IconID.h"
#include "IDPool.h"
#include "entt/signal/sigh.hpp"
#include "nlohmann/json_fwd.hpp"
#include <map>
#include <string>

struct SDL_Renderer;

namespace AM
{
namespace ResourceImporter
{

class DataModel;

/**
 * Holds data for icons and their parent icon sheets.
 */
class IconModel
{
public:
    IconModel(DataModel& inDataModel, SDL_Renderer* inSdlRenderer);

    /**
     * Attempts to load the "iconSheets" section of the given json into this 
     * model.
     *
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool load(const nlohmann::json& json);

    /**
     * Saves the current state of this data model into the given json's 
     * "iconSheets" section.
     */
    void save(nlohmann::json& json);

    /**
     * If the given data is valid, adds a new icon sheet to the back of the
     * iconSheets vector. All icons in the sheet will be extracted and
     * added to the sheet's icons vector.
     *
     * Note: We currently only support icon sheets with consistent icon
     *       sizes and no padding.
     *
     * @param relPath The path to the desired icon sheet, relative to
     *                the application's base directory.
     * @param iconWidth How wide each icon is.
     * @param iconHeight How tall each icon is.
     * @param baseName The name to prepend to each icon's number. (e.g.
     *                 "mob_" results in "mob_0", "mob_1", etc.)
     *
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool addIconSheet(const std::string& relPath,
                      const std::string& iconWidth,
                      const std::string& iconHeight,
                      const std::string& baseName);

    /**
     * Removes the icon sheet with the given ID from the icon sheet map.
     *
     * Errors if the given ID isn't present in iconSheetMap.
     *
     * @param sheetID The editor ID of the sheet to remove.
     */
    void remIconSheet(int sheetID);

    /**
     * Removes the icon with the given ID from the icon map.
     *
     * Errors if the given ID isn't present in iconMap.
     *
     * @param iconID The editor ID of the icon to remove.
     */
    void remIcon(IconID iconID);

    const EditorIcon& getIcon(IconID iconID);

    // Icon properties.
    void setIconDisplayName(IconID iconID, const std::string& newDisplayName);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    const std::string& getErrorString();

private:
    // Note: These were arbitrarily chosen and can be increased if necessary.
    static constexpr std::size_t MAX_ICON_SHEETS{1000};
    static constexpr std::size_t MAX_ICONS{MAX_ICON_SHEETS * 100};

    // Parsing functions.
    /**
     * @param sheetJson The json to parse. Must be a valid icon sheet section
     *                  from ResourceData.json.
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool parseIconSheet(const nlohmann::json& sheetJson);
    /**
     * @param iconJson The json to parse. Must be a valid icon section
     *                 from ResourceData.json.
     * @param iconSheet The sheet that this icon is from.
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool parseIcon(const nlohmann::json& iconJson, EditorIconSheet& iconSheet);

    /**
     * Checks if the given name is unique among all icons in the model.
     *
     * @param iconID The ID of the icon that might get displayName. If it
     *               already is set to displayName, it won't be counted as
     *               non-unique.
     * @param displayName The display name that the icon will be set to.
     */
    bool iconNameIsUnique(IconID iconID, const std::string& displayName);

    DataModel& dataModel;

    /** Used for validating user-selected sprite sheet textures. */
    SDL_Renderer* sdlRenderer;

    /** Maps sheet IDs -> the icon sheets that we currently have loaded. */
    std::map<int, EditorIconSheet> iconSheetMap;

    /** Maps icon IDs -> the icons that we currently have loaded. */
    std::map<IconID, EditorIcon> iconMap;

    /** Used for generating temporary icon IDs that are only used internally 
        by this editor. */
    IDPool sheetIDPool;
    IDPool iconIDPool;

    /** If one of our parsing functions returns false, this holds a string 
        describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(int sheetID, const EditorIconSheet& sheet)>
        sheetAddedSig;
    entt::sigh<void(int sheetID)> sheetRemovedSig;
    entt::sigh<void(IconID iconID)> iconRemovedSig;

    entt::sigh<void(IconID iconID, const std::string& newDisplayName)>
        iconDisplayNameChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A icon sheet was added to the model. */
    entt::sink<entt::sigh<void(int sheetID, const EditorIconSheet& sheet)>>
        sheetAdded;
    /** A icon sheet was removed from the model. */
    entt::sink<entt::sigh<void(int sheetID)>> sheetRemoved;
    /** A icon was removed from the model. */
    entt::sink<entt::sigh<void(IconID iconID)>> iconRemoved;

    /** A icon's display name has changed. */
    entt::sink<
        entt::sigh<void(IconID iconID, const std::string& newDisplayName)>>
        iconDisplayNameChanged;
};

} // namespace ResourceImporter
} // namespace AM
