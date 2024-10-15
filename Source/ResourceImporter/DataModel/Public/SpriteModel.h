#pragma once

#include "EditorSpriteSheet.h"
#include "EditorSprite.h"
#include "SpriteSheetID.h"
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

/**
 * Holds data for sprites and their parent sprite sheets.
 */
class SpriteModel
{
public:
    SpriteModel(DataModel& inDataModel, SDL_Renderer* inSdlRenderer);

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
     * Adds an empty sprite sheet to the back of the spriteSheets vector.
     */
    bool addSpriteSheet();

    /**
     * Removes the sprite sheet with the given ID from the sprite sheet map.
     *
     * Errors if the given ID isn't present in spriteSheetMap.
     *
     * @param sheetID The editor ID of the sheet to remove.
     */
    void remSpriteSheet(SpriteSheetID sheetID);

    /**
     * Adds a sprite with the given image.
     * The sprite's name will be derived from its filename. If the name ends in
     * "_<n>", the new sprite will also be added to an Animation.
     *
     * @param imageRelPath The relative path to the sprite's individual image 
     *                     file.
     *
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool addSprite(const std::string& imageRelPath,
                   SpriteSheetID parentSheetID);

    /**
     * Removes the sprite with the given ID from the sprite map.
     *
     * Errors if the given ID isn't present in spriteMap.
     *
     * @param spriteID The editor ID of the sprite to remove.
     */
    void remSprite(SpriteID spriteID);

    const EditorSpriteSheet& getSpriteSheet(SpriteSheetID sheetID) const;
    const EditorSprite& getSprite(SpriteID spriteID) const;

    // Sprite sheet properties.
    void setSpriteSheetDisplayName(SpriteSheetID spriteSheetID,
                                   const std::string& newDisplayName);

    // Sprite properties.
    // Note: We don't offer a setter for DisplayName because it should always 
    //       be based on the image filename.
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

    // Note: These are named differently to disambiguate them, since they aren't 
    //       accessible outside of this class.
    EditorSpriteSheet& mgetSpriteSheet(SpriteSheetID sheetID);
    EditorSprite& mgetSprite(SpriteID spriteID);

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
     * Checks if the given name is unique among all sprite sheets in the model.
     *
     * @param spriteSheetID The ID of the sprite sheet that might get 
     *                      displayName. If it already is set to displayName, 
     *                      it won't be counted as non-unique.
     * @param displayName The display name that the sprite will be set to.
     */
    bool spriteSheetNameIsUnique(SpriteSheetID spriteSheetID,
                                 const std::string& displayName);

    /**
     * Checks if the given name is unique among all sprites in the model.
     *
     * @param spriteID The ID of the sprite that might get displayName. If it
     *                 already is set to displayName, it won't be counted as
     *                 non-unique.
     * @param displayName The display name that the sprite will be set to.
     */
    bool spriteNameIsUnique(SpriteID spriteID, const std::string& displayName);

    /**
     * Sets the texture position of each sprite in the given sheet, and sets the 
     * sheet's size.
     */
    void refreshSpriteSheet(EditorSpriteSheet& spriteSheet);

    /**
     * If the given sprite is named such that it should be added to an 
     * animation, adds it. If the animation doesn't already exist, it will be 
     * created.
     */
    void addSpriteToAnimationIfNecessary(const EditorSprite& sprite);

    /**
     * If the given sprite is in an animation, removes it.
     */
    void remSpriteFromAnimationIfNecessary(const EditorSprite& sprite);

    /**
     * Returns a frame number from the end of the given sprite display name,
     * if one is present.
     * If the file name has no trailing frame number, returns -1.
     */
    int getFrameNumber(const std::string& displayName);

    /**
     * Derives an Animation display name from a given Sprite's display name by 
     * removing the trailing frame number.
     */
    std::string_view deriveAnimationName(std::string_view spriteDisplayName);

    DataModel& dataModel;

    /** Used for validating user-selected sprite sheet textures. */
    SDL_Renderer* sdlRenderer;

    /** Maps sheet IDs -> the sprite sheets that we currently have loaded. */
    std::map<SpriteSheetID, EditorSpriteSheet> spriteSheetMap;

    /** Maps sprite IDs -> the sprites that we currently have loaded. */
    std::map<SpriteID, EditorSprite> spriteMap;

    /** Maps sprite sheet names -> their ID. */
    std::unordered_map<std::string, SpriteSheetID> spriteSheetNameMap;

    /** Maps sprite names -> their ID. */
    std::unordered_map<std::string, SpriteID> spriteNameMap;

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
    entt::sigh<void(SpriteSheetID sheetID, const EditorSpriteSheet& sheet)>
        sheetAddedSig;
    entt::sigh<void(SpriteSheetID sheetID)> sheetRemovedSig;
    entt::sigh<void(SpriteID spriteID, const EditorSprite& sprite,
                    SpriteSheetID parentSheetID)>
        spriteAddedSig;
    entt::sigh<void(SpriteID spriteID, SpriteSheetID parentSheetID)>
        spriteRemovedSig;

    entt::sigh<void(SpriteSheetID spriteSheetID,
                    const std::string& newDisplayName)>
        spriteSheetDisplayNameChangedSig;

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
    entt::sink<
        entt::sigh<void(SpriteSheetID sheetID, const EditorSpriteSheet& sheet)>>
        sheetAdded;
    /** A sprite sheet was removed from the model. */
    entt::sink<entt::sigh<void(SpriteSheetID sheetID)>> sheetRemoved;
    /** A sprite was added to from the model. */
    entt::sink<entt::sigh<void(SpriteID spriteID, const EditorSprite& sprite,
                               SpriteSheetID parentSheetID)>>
        spriteAdded;
    /** A sprite was removed from the model. */
    entt::sink<entt::sigh<void(SpriteID spriteID, SpriteSheetID parentSheetID)>>
        spriteRemoved;

    /** A sprite sheet's display name has changed. */
    entt::sink<entt::sigh<void(SpriteSheetID spriteSheetID,
                               const std::string& newDisplayName)>>
        spriteSheetDisplayNameChanged;

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
