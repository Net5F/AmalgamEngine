#pragma once

#include "SpriteSets.h"
#include "LibraryItemData.h"
#include "SpriteModel.h"
#include "SpriteSetModel.h"
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
 * A data model that holds our resource data in memory and supports saving it 
 * into a ResourceData.json file.
 *
 * Note: All ID spaces are separate (i.e. sprites, sprite sheets, floors, etc 
 *       all have unique ID spaces).
 * Note: The sprite numeric ID space might accrue gaps as sprite are deleted.
 *       This is known and expected. Sprites are ordered in the UI by their 
 *       sheet's ID, so it shouldn't inconvenience users.
 */
class DataModel
{
public:
    DataModel(SDL_Renderer* inSdlRenderer);

    // Note: We put SpriteSetModel first because SpriteModel is dependent.
    /** Holds Floor, FloorCovering, Wall, and Object SpriteSet data. */
    SpriteSetModel spriteSetModel;

    /** Holds Sprite and SpriteSheet data. */
    SpriteModel spriteModel;

    /**
     * Creates a new ResourceData.json file at the given path and saves to it.
     *
     * @param fullPath  The full path to the directory where ResourceData.json
     *                  should be created.
     * @return An empty string if the file at the given path parses
     *         successfully, else a string containing the failure message.
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool create(const std::string& fullPath);

    /**
     * Attempts to open the ResourceData.json at the given path, parse it,
     * and load the data into this model.
     *
     * @param fullPath  The full path to the ResourceData.json file.
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool load(const std::string& fullPath);

    /**
     * Saves the current state of this data model into the ResourceData.json
     * file at the previously loaded path.
     */
    void save();

    /** Sets the current active library item to the given sprite. */
    void setActiveSprite(int newActiveSpriteID);

    /** Sets the current active library item to the given sprite set. */
    void setActiveSpriteSet(SpriteSet::Type type, Uint16 newActiveSpriteSetID);

    const std::string& getWorkingTexturesDir();

    const std::string& getErrorString();

    /**
     * Derives a string ID from a display name by making it all lowercase and
     * replacing spaces with underscores.
     */
    static std::string deriveStringID(const std::string& displayName);

    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** The active library item has changed. */
    entt::sink<entt::sigh<void(const LibraryItemData& newActiveItem)>>
        activeLibraryItemChanged;

private:
    /**
     * Sets currentWorkingDir to the parent directory of
     * currentWorkingFilePath + "/Assets/Textures".
     */
    bool setWorkingTexturesDir();

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    /** The ResourceData.json file that we currently have loaded and are working
        on. */
    std::string workingFilePath;

    /** The parent directory of currentWorkingFilePath + "/Assets/Textures". */
    std::string workingTexturesDir;

    /** If one of our member model's load functions returns false, this holds a 
        string describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(const LibraryItemData& newActiveItem)>
        activeLibraryItemChangedSig;
};

} // namespace SpriteEditor
} // namespace AM
