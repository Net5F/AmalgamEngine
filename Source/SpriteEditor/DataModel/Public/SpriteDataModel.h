#pragma once

#include "EditorSpriteSheet.h"
#include "EditorSprite.h"
#include "SpriteSets.h"
#include "BoundingBox.h"
#include "LibraryItemData.h"
#include "IDPool.h"
#include "entt/signal/sigh.hpp"
#include <fstream>
#include <map>
#include <string>

struct SDL_Renderer;

namespace AM
{
namespace SpriteEditor
{
/**
 * A data model that holds our SpriteData.json project file in memory.
 *
 * Note: All ID spaces are separate (i.e. sprites, sprite sheets, floors, etc 
 *       all have unique ID spaces.
 *       Additionally, the IDs used at runtime are temporary. We generate fresh
 *       IDs while saving the json, to make sure we don't accumulate any gaps 
 *       in the ID sequence.
 */
class SpriteDataModel
{
public:
    SpriteDataModel(SDL_Renderer* inSdlRenderer);

    /**
     * Creates a new SpriteData.json file at the given path and saves to it.
     *
     * @param fullPath  The full path to the directory where SpriteData.json
     *                  should be created.
     * @return An empty string if the file at the given path parses
     *         successfully, else a string containing the failure message.
     */
    std::string create(const std::string& fullPath);

    /**
     * Attempts to open the SpriteData.json at the given path, parse it,
     * and load the data into this model.
     *
     * @param fullPath  The full path to the SpriteData.json file.
     * @return An empty string if the file at the given path parses
     *         successfully, else a string containing the failure message.
     */
    std::string load(const std::string& fullPath);

    /**
     * Saves the current state of this data model into the SpriteData.json
     * file at the previously loaded path.
     *
     * @pre parse() must have been called to load a SpriteData.json.
     */
    void save();

    /**
     * If the given data is valid, adds a new sprite sheet to the back of the
     * spriteSheets vector. All sprites in the sheet will be extracted and
     * added to the sheet's sprites vector.
     *
     * Note: We currently only support sprite sheets with consistent sprite
     *       sizes and no padding.
     *
     * @param relPath  The path to the desired sprite sheet, relative to
     *                 the application's base directory.
     * @param spriteWidth  How wide each sprite is.
     * @param spriteHeight  How tall each sprite is.
     * @param yOffset  How much each sprite should be offset in the Y direction
     *                 to line up with their tile.
     * @param baseName  The name to prepend to each sprite's number. (e.g.
     *                  "mob_" results in "mob_0", "mob_1", etc.)
     *
     * @return "" if the given data is valid, else an appropriate error string.
     */
    std::string addSpriteSheet(const std::string& relPath,
                               const std::string& spriteWidth,
                               const std::string& spriteHeight,
                               const std::string& yOffset,
                               const std::string& baseName);

    /**
     * Removes the sprite sheet with the given ID from the sprite sheet map.
     *
     * Errors if the given ID isn't present in spriteSheetMap.
     *
     * @param sheetID  The editor ID of the sheet to be removed.
     */
    void remSpriteSheet(int sheetID);

    /**
     * Removes the sprite with the given ID from the sprite map.
     *
     * Errors if the given ID isn't present in spriteMap.
     *
     * @param spriteID  The editor ID of the sprite to be removed.
     */
    void remSprite(int spriteID);

    const EditorSprite& getSprite(int spriteID);

    const std::string& getWorkingTexturesDir();

    //-------------------------------------------------------------------------
    // Active Sprite
    //-------------------------------------------------------------------------
    void setActiveSprite(int newActiveSpriteID);

    void setActiveSpriteSet(SpriteSet::Type type, Uint16 newActiveSpriteSetID);

    void setSpriteDisplayName(int spriteID,
                              const std::string& newDisplayName);

    void setSpriteCollisionEnabled(int spriteID,
                                   bool newCollisionEnabled);

    void setSpriteModelBounds(int spriteID,
                              const BoundingBox& newModelBounds);

    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A sprite sheet was added to the model. */
    entt::sink<entt::sigh<void(int sheetID, const EditorSpriteSheet& sheet)>>
        sheetAdded;

    /** A sprite sheet was removed from the model. */
    entt::sink<entt::sigh<void(int sheetID)>> sheetRemoved;

    /** A sprite was removed from the model. */
    entt::sink<entt::sigh<void(int spriteID)>> spriteRemoved;

    /** The active library item has changed. */
    entt::sink<entt::sigh<void(const LibraryItemData& newActiveItem)>>
        activeLibraryItemChanged;

    /** A sprite's display name has changed. */
    entt::sink<entt::sigh<void(int spriteID,
                               const std::string& newDisplayName)>>
        spriteDisplayNameChanged;

    /** A sprite's "has bounding box" field has changed. */
    entt::sink<entt::sigh<void(int spriteID, bool collisionEnabled)>>
        spriteCollisionEnabledChanged;

    /** A sprite's bounding box has changed. */
    entt::sink<entt::sigh<void(int spriteID,
                               const BoundingBox& newModelBounds)>>
        spriteModelBoundsChanged;

private:
    // Note: These were arbitrarily chosen and can be increased if necessary.
    static constexpr std::size_t MAX_SPRITE_SHEETS{1000};
    static constexpr std::size_t MAX_SPRITES{MAX_SPRITE_SHEETS * 100};

    /**
     * Checks that the given relative path corresponds to a valid sprite
     * sheet image in the working Resources directory.
     *
     * @return "" if the given path is valid, else an appropriate error string.
     */
    std::string validateRelPath(const std::string& relPath);

    /**
     * Sets currentWorkingDir to the parent directory of
     * currentWorkingFilePath + "/Assets/Textures".
     */
    bool setWorkingTexturesDir();

    /**
     * Derives a string ID from a display name by making it all lowercase and
     * replacing spaces with underscores.
     */
    std::string deriveStringId(const std::string& displayName);

    /**
     * Checks if the given name is unique among all sprites in the model.
     *
     * @param spriteID  The ID of the sprite that might get displayName. If it
     *                  already is set to displayName, it won't be counted as
     *                  non-unique.
     * @param displayName  The display name that the sprite will be set to.
     */
    bool spriteNameIsUnique(int spriteID,
                            const std::string& displayName);

    /** Used for validating user-selected sprite sheet textures. */
    SDL_Renderer* sdlRenderer;

    /** The SpriteData.json file that we currently have loaded and are working
        on. */
    std::string workingFilePath;

    /** The parent directory of currentWorkingFilePath + "/Assets/Textures". */
    std::string workingTexturesDir;

    /** Maps sheet IDs -> the sprite sheets that we currently have loaded. */
    std::map<int, EditorSpriteSheet> spriteSheetMap;

    /** Maps sprite IDs -> the sprites that we currently have loaded. */
    std::map<int, EditorSprite> spriteMap;

    /** Maps floor IDs -> the floor sprite sets that we currently have loaded. */
    std::map<Uint16, EditorFloorSpriteSet> floorMap;

    /** Maps floor covering IDs -> the floor sprite sets that we currently have 
        loaded. */
    std::map<Uint16, EditorFloorCoveringSpriteSet> floorCoveringMap;

    /** Maps wall IDs -> the wall sprite sets that we currently have loaded. */
    std::map<Uint16, EditorWallSpriteSet> wallMap;

    /** Maps object IDs -> the object sprite sets that we currently have loaded. */
    std::map<Uint16, EditorObjectSpriteSet> objectMap;

    /** Used for generating temporary sprite sheet IDs that are only used
        internally by this editor. */
    IDPool sheetIDPool;

    /** Used for generating temporary sprite sheet IDs that are only used
        internally by this editor. */
    IDPool spriteIDPool;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(int sheetID, const EditorSpriteSheet& sheet)>
        sheetAddedSig;

    entt::sigh<void(int sheetID)> sheetRemovedSig;

    entt::sigh<void(int spriteID)> spriteRemovedSig;

    entt::sigh<void(const LibraryItemData& newActiveItem)>
        activeLibraryItemChangedSig;

    entt::sigh<void(int spriteID, const std::string& newDisplayName)>
        spriteDisplayNameChangedSig;

    entt::sigh<void(int spriteID, bool collisionEnabled)>
        spriteCollisionEnabledChangedSig;

    entt::sigh<void(int spriteID, const BoundingBox& newModelBounds)>
        spriteModelBoundsChangedSig;
};

} // namespace SpriteEditor
} // namespace AM
