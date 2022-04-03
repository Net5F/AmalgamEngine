#pragma once

#include "SpriteSheet.h"
#include "IDPool.h"
#include "entt/signal/sigh.hpp"
#include <fstream>
#include <vector>
#include <string>

struct SDL_Renderer;

namespace AM
{
namespace SpriteEditor
{
/**
 * A data model that holds our SpriteData.json project file in memory.
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
     *                   the application's base directory.
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
     * Removes the sprite sheet at the given index in the spriteSheets vector.
     *
     * Error if no sprite sheet is at the given index.
     */
    void remSpriteSheet(unsigned int index);

    void setActiveSprite();

    void setActiveSpriteName();

    /**
     * Checks if the given name is unique among all sprites in the model.
     */
    bool spriteNameIsUnique(const std::string& displayName);

    std::vector<SpriteSheet>& getSpriteSheets();
    const std::string& getWorkingTexturesDir();

    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A sprite sheet has been added to the model. */
    entt::sink<void(const SpriteSheet&)> sheetAdded;

    /** A sprite has been added to the model. */
    entt::sink<void(const Sprite&)> spriteAdded;

private:
    // Note: These were arbitrarily chosen and can be increased if necessary.
    static constexpr unsigned int MAX_SPRITE_SHEETS = 1000;
    static constexpr unsigned int MAX_SPRITES = MAX_SPRITE_SHEETS * 100;

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
     * Called once we've fully loaded a SpriteData.json file.
     * Signals all loaded sprite sheets and sprites to the UI.
     */
    void postLoadSendSignals();

    /** Used for validating user-selected sprite sheet textures. */
    SDL_Renderer* sdlRenderer;

    /** The SpriteData.json file that we currently have loaded and are working
        on. */
    std::string workingFilePath;

    /** The parent directory of currentWorkingFilePath + "/Assets/Textures". */
    std::string workingTexturesDir;

    /** The sprite sheets that we currently have loaded. */
    std::vector<SpriteSheet> spriteSheets;

    /** Used for generating temporary sprite sheet IDs that are only used
        internally by this editor. */
    IDPool sheetIDPool;

    /** Used for generating temporary sprite sheet IDs that are only used
        internally by this editor. */
    IDPool spriteIDPool;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(const SpriteSheet&)> sheetAddedSig;

    entt::sigh<void(const Sprite&)> spriteAddedSig;
};

} // namespace SpriteEditor
} // namespace AM
