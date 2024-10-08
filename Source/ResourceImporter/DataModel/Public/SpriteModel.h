#pragma once

#include "EditorSpriteSheet.h"
#include "EditorSprite.h"
#include "BoundingBox.h"
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
class GraphicSetModel;
class EntityGraphicSetModel;

/**
 * Holds data for sprites and their parent sprite sheets.
 */
class SpriteModel
{
public:
    SpriteModel(DataModel& inDataModel, GraphicSetModel& inGraphicSetModel,
                EntityGraphicSetModel& inEntityGraphicSetModel,
                SDL_Renderer* inSdlRenderer);

    /**
     * Attempts to load the "spriteSheets" section of the given json into this
     * model.
     *
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool load(const nlohmann::json& json);

    /**
     * Saves the current state of this data model into the given json's
     * "spriteSheets" section.
     */
    void save(nlohmann::json& json);

    /**
     * If the given data is valid, adds a new sprite sheet to the back of the
     * spriteSheets vector. All sprites in the sheet will be extracted and
     * added to the sheet's sprites vector.
     *
     * Note: We currently only support sprite sheets with consistent sprite
     *       sizes and no padding.
     *
     * @param relPath The path to the desired sprite sheet, relative to
     *                the application's base directory.
     * @param spriteWidth How wide each sprite is.
     * @param spriteHeight How tall each sprite is.
     * @param yOffset How much each sprite should be offset in the Y direction
     *                to line up with their tile.
     * @param baseName The name to prepend to each sprite's number. (e.g.
     *                 "mob_" results in "mob_0", "mob_1", etc.)
     *
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool addSpriteSheet(const std::string& relPath,
                        const std::string& spriteWidth,
                        const std::string& spriteHeight,
                        const std::string& stageOriginX,
                        const std::string& stageOriginY,
                        const std::string& baseName);

    /**
     * Removes the sprite sheet with the given ID from the sprite sheet map.
     *
     * Errors if the given ID isn't present in spriteSheetMap.
     *
     * @param sheetID The editor ID of the sheet to remove.
     */
    void remSpriteSheet(int sheetID);

    /**
     * Removes the sprite with the given ID from the sprite map.
     *
     * Errors if the given ID isn't present in spriteMap.
     *
     * @param spriteID The editor ID of the sprite to remove.
     */
    void remSprite(SpriteID spriteID);

    const EditorSprite& getSprite(SpriteID spriteID);

    // Sprite properties.
    void setSpriteDisplayName(SpriteID spriteID,
                              const std::string& newDisplayName);
    void setSpriteModelBoundsID(SpriteID spriteID,
                                BoundingBoxID newModelBoundsID);
    void setSpriteCustomModelBounds(SpriteID spriteID,
                                    const BoundingBox& newModelBounds);
    void setSpriteCollisionEnabled(SpriteID spriteID, bool newCollisionEnabled);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    const std::string& getErrorString();

private:
    // Note: These were arbitrarily chosen and can be increased if necessary.
    static constexpr std::size_t MAX_SPRITE_SHEETS{1000};
    static constexpr std::size_t MAX_SPRITES{MAX_SPRITE_SHEETS * 100};

    // Parsing functions.
    /**
     * @param sheetJson The json to parse. Must be a valid sprite sheet section
     *                  from ResourceData.json.
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool parseSpriteSheet(const nlohmann::json& sheetJson);
    /**
     * @param spriteJson The json to parse. Must be a valid sprite section
     *                   from ResourceData.json.
     * @param spriteSheet The sheet that this sprite is from.
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool parseSprite(const nlohmann::json& spriteJson,
                     EditorSpriteSheet& spriteSheet);

    /**
     * Checks if the given name is unique among all sprites in the model.
     *
     * @param spriteID The ID of the sprite that might get displayName. If it
     *                 already is set to displayName, it won't be counted as
     *                 non-unique.
     * @param displayName The display name that the sprite will be set to.
     */
    bool spriteNameIsUnique(SpriteID spriteID, const std::string& displayName);

    DataModel& dataModel;
    /** Used to remove graphics from graphic sets when a graphic is deleted. */
    GraphicSetModel& graphicSetModel;
    EntityGraphicSetModel& entityGraphicSetModel;

    /** Used for validating user-selected sprite sheet textures. */
    SDL_Renderer* sdlRenderer;

    /** Maps sheet IDs -> the sprite sheets that we currently have loaded. */
    std::map<int, EditorSpriteSheet> spriteSheetMap;

    /** Maps sprite IDs -> the sprites that we currently have loaded. */
    std::map<SpriteID, EditorSprite> spriteMap;

    /** Used for generating temporary sprite sheet IDs that are only used
        internally by this editor. */
    IDPool sheetIDPool;
    IDPool spriteIDPool;

    /** If one of our parsing functions returns false, this holds a string
        describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(int sheetID, const EditorSpriteSheet& sheet)> sheetAddedSig;
    entt::sigh<void(int sheetID)> sheetRemovedSig;
    entt::sigh<void(SpriteID spriteID)> spriteRemovedSig;

    entt::sigh<void(SpriteID spriteID, const std::string& newDisplayName)>
        spriteDisplayNameChangedSig;
    entt::sigh<void(SpriteID spriteID, BoundingBoxID newModelBoundsID)>
        spriteModelBoundsIDChangedSig;
    entt::sigh<void(SpriteID spriteID, const BoundingBox& newCustomModelBounds)>
        spriteCustomModelBoundsChangedSig;
    entt::sigh<void(SpriteID spriteID, bool newCollisionEnabled)>
        spriteCollisionEnabledChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A sprite sheet was added to the model. */
    entt::sink<entt::sigh<void(int sheetID, const EditorSpriteSheet& sheet)>>
        sheetAdded;
    /** A sprite sheet was removed from the model. */
    entt::sink<entt::sigh<void(int sheetID)>> sheetRemoved;
    /** A sprite was removed from the model. */
    entt::sink<entt::sigh<void(SpriteID spriteID)>> spriteRemoved;

    /** A sprite's display name has changed. */
    entt::sink<
        entt::sigh<void(SpriteID spriteID, const std::string& newDisplayName)>>
        spriteDisplayNameChanged;
    /** A sprite's bounding box ID has changed. */
    entt::sink<
        entt::sigh<void(SpriteID spriteID, BoundingBoxID newModelBoundsID)>>
        spriteModelBoundsIDChanged;
    /** A sprite's custom bounding box has changed. */
    entt::sink<entt::sigh<void(SpriteID spriteID,
                               const BoundingBox& newCustomModelBounds)>>
        spriteCustomModelBoundsChanged;
    /** A sprite's "collision enabled" field has changed. */
    entt::sink<entt::sigh<void(SpriteID spriteID, bool newCollisionEnabled)>>
        spriteCollisionEnabledChanged;
};

} // namespace ResourceImporter
} // namespace AM
