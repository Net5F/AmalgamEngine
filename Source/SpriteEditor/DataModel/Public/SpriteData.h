#pragma once

#include <filesystem>

namespace AM
{
namespace SpriteEditor
{

/**
 * A data model that holds our SpriteData.json project file in memory.
 */
class SpriteData
{
public:
    SpriteData();

    /**
     * Attempts to open the SpriteData.json at the given path, parse it,
     * and load the data into this model.
     *
     * @return true if the file at the given path parses successfully, else
     *         false.
     */
    bool parse(std::filesystem::path path);

    /**
     * Saves the current state of this data model into the SpriteData.json
     * file at the previously loaded path.
     *
     * @pre load() must have been called to load a SpriteData.json.
     */
    void save();

private:
};

} // namespace SpriteEditor
} // namespace AM
