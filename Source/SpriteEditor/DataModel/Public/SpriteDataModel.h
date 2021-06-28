#pragma once

#include "SpriteSheet.h"
#include <fstream>
#include <vector>
#include <string>

namespace SDL2pp
{
    class Renderer;
} // namespace SDL2pp

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
    SpriteDataModel(SDL2pp::Renderer& inSdlRenderer);

    /**
     * Creates a new SpriteData.json file at the given path and saves to it.
     *
     * @param fullPath  The full path to the directory where SpriteData.json
     *                  should be created.
     * @return false if the given path already has a SpriteData.json, or file
     *         creation fails.
     */
    bool create(const std::string& fullPath);

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
     * @param baseName  The name to prepend to each sprite's number. (e.g.
     *                  "mob_" results in "mob_0", "mob_1", etc.)
     *
     * @return "" if the given data is valid, else an appropriate error string.
     */
    std::string addSpriteSheet(const std::string& relPath, const std::string& spriteWidth
                               , const std::string& spriteHeight, const std::string& baseName);

    const std::vector<SpriteSheet>& getSpriteSheets();

private:
    /** Used for validating user-selected sprite sheet textures. */
    SDL2pp::Renderer& sdlRenderer;

    /** The sprite sheets that we currently have loaded. */
    std::vector<SpriteSheet> spriteSheets;

    /** The file that we currently have loaded and are working on. */
    std::fstream currentWorkingFile;
};

} // namespace SpriteEditor
} // namespace AM
