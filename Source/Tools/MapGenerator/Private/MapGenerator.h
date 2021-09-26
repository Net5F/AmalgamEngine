#pragma once

#include "NetworkDefs.h"
#include <string>
#include <cstdint>
#include <vector>

namespace AM
{
namespace MG
{
/**
 * Generates the World.map file based on the given parameters.
 */
class MapGenerator
{
public:
    MapGenerator(unsigned int inMapLengthX, unsigned int inMapLengthY,
                 const std::string& inFillSpriteId);

    /**
     * Generates the map and saves it to a file with the given name, placed in
     * the same directory as the program binary.
     */
    void generateAndSave(const std::string& fileName);

private:
    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr uint16_t MAP_FORMAT_VERSION = 0;

    /** The length, in chunks, of the map's X axis. */
    unsigned int mapXLength;

    /** The length, in chunks, of the map's Y axis. */
    unsigned int mapYLength;

    /** The ID of the sprite to fill the map with. */
    std::string fillSpriteId;

    /** The string that we store the generated map data in. */
    BinaryBuffer mapData;

    /** Used to store the size of the serialized data, once mapData is
        filled. */
    unsigned long int dataSize;
};

} // End namespace MG
} // End namespace AM
