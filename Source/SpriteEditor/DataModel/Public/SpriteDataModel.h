#pragma once

#include "EditorSpriteSheet.h"
#include "EditorSprite.h"
#include "SpriteSets.h"
#include "BoundingBox.h"
#include "LibraryItemData.h"
#include "IDPool.h"
#include "entt/signal/sigh.hpp"
#include "nlohmann/json_fwd.hpp"
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
 * Note: The sprite numeric ID space might accrue gaps as sprite are deleted.
 *       The is known and expected. Sprites are ordered in the UI by their 
 *       sheet's ID, so it shouldn't inconvenience users.
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
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool create(const std::string& fullPath);

    /**
     * Attempts to open the SpriteData.json at the given path, parse it,
     * and load the data into this model.
     *
     * @param fullPath  The full path to the SpriteData.json file.
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool load(const std::string& fullPath);

    /**
     * Saves the current state of this data model into the SpriteData.json
     * file at the previously loaded path.
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
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool addSpriteSheet(const std::string& relPath,
                        const std::string& spriteWidth,
                        const std::string& spriteHeight,
                        const std::string& yOffset,
                        const std::string& baseName);

    /**
     * Removes the sprite sheet with the given ID from the sprite sheet map.
     *
     * Errors if the given ID isn't present in spriteSheetMap.
     *
     * @param sheetID  The editor ID of the sheet to remove.
     */
    void remSpriteSheet(int sheetID);

    /**
     * Removes the sprite with the given ID from the sprite map.
     *
     * Errors if the given ID isn't present in spriteMap.
     *
     * @param spriteID  The editor ID of the sprite to remove.
     */
    void remSprite(int spriteID);

    const EditorSprite& getSprite(int spriteID);

    /**
     * Adds a blank sprite set of the appropriate type and loads it.
     */
    bool addFloor();
    bool addFloorCovering();
    bool addWall();
    bool addObject();

    /**
     * Removes the sprite set of the appropriate type with the given ID from 
     * the associated map.
     * 
     * Error if the given ID isn't preset in the map.
     *
     * @param spriteSetID  The editor ID of the sprite set to remove.
     */
    void remFloor(Uint16 floorID);
    void remFloorCovering(Uint16 floorCoveringID);
    void remWall(Uint16 wallID);
    void remObject(Uint16 objectID);

    /** Sets the current active library item to the given sprite. */
    void setActiveSprite(int newActiveSpriteID);

    /** Sets the current active library item to the given sprite set. */
    void setActiveSpriteSet(SpriteSet::Type type, Uint16 newActiveSpriteSetID);

    // Sprite properties.
    void setSpriteDisplayName(int spriteID,
                              const std::string& newDisplayName);
    void setSpriteCollisionEnabled(int spriteID,
                                   bool newCollisionEnabled);
    void setSpriteModelBounds(int spriteID,
                              const BoundingBox& newModelBounds);

    // Sprite set properties.
    void setSpriteSetDisplayName(SpriteSet::Type type, Uint16 spriteSetID,
                                 const std::string& newDisplayName);

    const std::string& getWorkingTexturesDir();

    const std::string& getErrorString();

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
    /** A sprite's "collision enabled" field has changed. */
    entt::sink<entt::sigh<void(int spriteID, bool collisionEnabled)>>
        spriteCollisionEnabledChanged;
    /** A sprite's bounding box has changed. */
    entt::sink<entt::sigh<void(int spriteID,
                               const BoundingBox& newModelBounds)>>
        spriteModelBoundsChanged;

    /** A floor sprite set was added to the model. */
    entt::sink<entt::sigh<void(Uint16 floorID, const EditorFloorSpriteSet& floor)>>
        floorAdded;
    /** A floor covering sprite set was added to the model. */
    entt::sink<entt::sigh<void(Uint16 floorCoveringID,
                               const EditorFloorCoveringSpriteSet& floorCovering)>>
        floorCoveringAdded;
    /** A wall sprite set was added to the model. */
    entt::sink<
        entt::sigh<void(Uint16 wallID, const EditorWallSpriteSet& wall)>>
        wallAdded;
    /** An object sprite set was added to the model. */
    entt::sink<
        entt::sigh<void(Uint16 objectID, const EditorObjectSpriteSet& floor)>>
        objectAdded;

    /** A sprite set was removed from the model. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID)>>
        spriteSetRemoved;

    /** A sprite set's sprite at the given index was changed. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               std::size_t index, int newSpriteID)>>
        spriteSetSlotChanged;

    /** A sprite set's display name has changed. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               const std::string& newDisplayName)>>
        spriteSetDisplayNameChanged;

private:
    // Note: These were arbitrarily chosen and can be increased if necessary.
    static constexpr std::size_t MAX_SPRITE_SHEETS{1000};
    static constexpr std::size_t MAX_SPRITES{MAX_SPRITE_SHEETS * 100};

    /**
     * Checks that the given relative path corresponds to a valid sprite
     * sheet image in the working Resources directory.
     *
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool validateRelPath(const std::string& relPath);

    /**
     * Sets currentWorkingDir to the parent directory of
     * currentWorkingFilePath + "/Assets/Textures".
     */
    bool setWorkingTexturesDir();

    /**
     * Derives a string ID from a display name by making it all lowercase and
     * replacing spaces with underscores.
     */
    std::string deriveStringID(const std::string& displayName);

    // Parsing functions.
    /**
     * @param sheetJson  The json to parse. Must be a valid sprite sheet section
     *                    from SpriteData.json.
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool parseSpriteSheet(const nlohmann::json& sheetJson);
    /**
     * @param spriteJson  The json to parse. Must be a valid sprite section
     *                    from SpriteData.json.
     * @param spriteSheet  The sheet that this sprite is from.
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool parseSprite(const nlohmann::json& spriteJson,
                     EditorSpriteSheet& spriteSheet);
    /**
     * @param spriteSetJson  The json to parse. Must be a valid sprite set 
     *                       section from SpriteData.json, for the appropriate 
     *                       set type.
     */
    bool parseFloorSpriteSet(const nlohmann::json& spriteSetJson);
    bool parseFloorCoveringSpriteSet(const nlohmann::json& spriteSetJson);
    bool parseWallSpriteSet(const nlohmann::json& spriteSetJson);
    bool parseObjectSpriteSet(const nlohmann::json& spriteSetJson);

    /**
     * Returns the appropriate map for the given sprite set type.
     */
    template<typename T>
    std::map<Uint16, T>& getMapForSpriteSetType();

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
    template<typename T>
    bool spriteSetNameIsUnique(Uint16 spriteSetID,
                               const std::string& displayName);

    /**
     * Implementation for setSpriteSetDisplayName().
     */
    template<typename T>
    void setSpriteSetDisplayName(SpriteSet::Type type, Uint16 spriteSetID,
                                 const std::string& newDisplayName);

    // Save functions.
    void saveSpriteSheets(nlohmann::json& json);
    void saveFloors(nlohmann::json& json);
    void saveFloorCoverings(nlohmann::json& json);
    void saveWalls(nlohmann::json& json);
    void saveObjects(nlohmann::json& json);

    /**
     * Iterates all the sprite sets and replaces any instances of spriteID 
     * with EMPTY_SPRITE_ID.
     */
    void removeSpriteIDFromSets(int spriteID);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

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
    IDPool spriteIDPool;
    IDPool floorIDPool;
    IDPool floorCoveringIDPool;
    IDPool wallIDPool;
    IDPool objectIDPool;

    /** If one of our parsing functions returns false, this holds a string 
        describing the error that occurred. */
    std::string errorString;

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

    entt::sigh<void(Uint16 floorID, const EditorFloorSpriteSet& floor)>
        floorAddedSig;
    entt::sigh<void(Uint16 floorCoveringID,
                    const EditorFloorCoveringSpriteSet& floorCovering)>
        floorCoveringAddedSig;
    entt::sigh<void(Uint16 wallID, const EditorWallSpriteSet& wall)>
        wallAddedSig;
    entt::sigh<void(Uint16 objectID, const EditorObjectSpriteSet& object)>
        objectAddedSig;

    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID)>
        spriteSetRemovedSig;
    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID, std::size_t index,
                    int newSpriteID)>
        spriteSetSlotChangedSig;
    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               const std::string& newDisplayName)>
        spriteSetDisplayNameChangedSig;
};

} // namespace SpriteEditor
} // namespace AM
