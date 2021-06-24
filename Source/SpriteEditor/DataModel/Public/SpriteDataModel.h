#pragma once

#include "SpriteSheet.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

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
    /**
     * Creates a new SpriteData.json file at the given path and saves to it.
     *
     * @return false if the given path already has a SpriteData.json, or file
     *         creation fails.
     */
    bool create(std::filesystem::path filePath);

    /**
     * Attempts to open the SpriteData.json at the given path, parse it,
     * and load the data into this model.
     *
     * @return An empty string if the file at the given path parses
     *         successfully, else a string containing the failure message.
     */
    std::string load(std::filesystem::path filePath);

    /**
     * Saves the current state of this data model into the SpriteData.json
     * file at the previously loaded path.
     *
     * @pre parse() must have been called to load a SpriteData.json.
     */
    void save();

private:
    /** The sprite sheets that we currently have loaded. */
    std::vector<SpriteSheet> spriteSheets;

    /** The file that we currently have loaded and are working on. */
    std::fstream currentWorkingFile;
};

} // namespace SpriteEditor
} // namespace AM
