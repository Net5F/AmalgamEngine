#include <SDL.h>

#include "MapGenerator.h"
#include "Timer.h"
#include "Ignore.h"

#include <iostream>

using namespace AM;
using namespace AM::MG;

void printUsage()
{
    std::printf(
        "Usage: MapGenerator.exe <XLength> <YLength> <FillSpriteId>\n"
        "  XLength: The map's x-axis length in chunks.\n"
        "  YLength: The map's y-axis length in chunks.\n"
        "  FillSpriteId: The string ID of the sprite to fill the map with.\n");
    std::fflush(stdout);
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::printf("Too few arguments.\n");
        printUsage();
        return 1;
    }

    // Parse map X length.
    char* end;
    int mapLengthX = std::strtol(argv[1], &end, 10);
    if ((*end != '\0') || (mapLengthX < 1)) {
        // Input didn't parse into an integer, or was an invalid number.
        std::printf("Invalid XLength: %s\n", argv[1]);
        printUsage();
        return 1;
    }

    // Parse map Y length.
    int mapLengthY = std::strtol(argv[2], &end, 10);
    if ((*end != '\0') || (mapLengthY < 1)) {
        // Input didn't parse into an integer, or was an invalid number.
        std::printf("Invalid YLength: %s\n", argv[2]);
        printUsage();
        return 1;
    }

    // Parse fill sprite ID.
    std::string fillSpriteId{argv[3]};

    // Prime a timer.
    Timer timer;
    timer.updateSavedTime();

    // Generate the map and save it.
    double startTime{timer.getDeltaSeconds(true)};
    MapGenerator mapGenerator(static_cast<unsigned int>(mapLengthX),
                              static_cast<unsigned int>(mapLengthY),
                              fillSpriteId);
    mapGenerator.generateAndSave("TileMap.bin");

    double timeTaken = timer.getDeltaSeconds(false) - startTime;
    std::printf("Map generated and saved in %.6fs.\n", timeTaken);
    std::fflush(stdout);

    return 0;
}
