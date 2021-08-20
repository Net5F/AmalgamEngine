#include <SDL2/SDL.h>

#include "MapGenerator.h"
#include "Ignore.h"

#include <iostream>

using namespace AM::MG;

void printUsage()
{
    std::printf("Usage: MapGenerator.exe <XLength> <YLength> <FillSpriteId>\n"
             "  XLength: The map's x-axis length in tiles. Must be a multiple of 16.\n"
             "  YLength: The map's y-axis length in tiles. Must be a multiple of 16.\n"
             "  FillSpriteId: The string ID of the sprite to fill the map with.");
    std::fflush(stdout);
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::printf("Too few arguments.\n");
        printUsage();
        return 1;
    }

    // Parse map width.
    char* end;
    int mapXLength = std::strtol(argv[1], &end, 10);
    if ((*end != '\0') || (mapXLength < 1) || (mapXLength % 16 != 0)) {
        // Input didn't parse into an integer, or was an invalid number.
        std::printf("Invalid XLength: %s\n", argv[1]);
        printUsage();
        return 1;
    }

    // Parse map height.
    int mapYLength = std::strtol(argv[2], &end, 10);
    if ((*end != '\0') || (mapYLength < 1) || (mapYLength % 16 != 0)) {
        // Input didn't parse into an integer, or was an invalid number.
        std::printf("Invalid YLength: %s\n", argv[2]);
        printUsage();
        return 1;
    }

    // Parse fill sprite ID.
    std::string fillSpriteId{argv[3]};

    // Generate the map and save it.
    MapGenerator mapGenerator(static_cast<unsigned int>(mapXLength)
                              , static_cast<unsigned int>(mapYLength), fillSpriteId);
    mapGenerator.generate();
    mapGenerator.save("World.map");

    return 0;
}
