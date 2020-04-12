#include <TileManager.h>
#include <AssetManager.h>
#include <GameManager.h>

void NW::TileManager::BuildTileTypes(const std::string& FileName)
{
    const AssetManager& assetManager = gameManager.GetAssetManager();

    // TODO: Replace this hardcoding with parsing the .desc file.
    const SDL2pp::Texture& ultimaTexture
            = assetManager.GetTexture("u4_tiles_pc_ega.png");

    // Tile 0
    tileTypes.push_back(Tile("WoodFloor", true, ultimaTexture
                             , SDL2pp::Rect(240, 48, 16, 16)));

    // Tile 1
    tileTypes.push_back(Tile("BrickWall", true, ultimaTexture
                             , SDL2pp::Rect(240, 112, 16, 16)));

    // Tile 2
    tileTypes.push_back(Tile("Water", true, ultimaTexture
                             , SDL2pp::Rect(16, 0, 16, 16)));
}

void NW::TileManager::BuildTileMap(const std::string& FileName)
{
    // TODO: Replace this hardcoding with parsing the .map file.
    mapSize = {9, 16};

    mapPtr = std::make_unique<Tile*[]>(mapSize.columnVal * mapSize.rowVal);

    for (int i = 0; i < mapSize.rowVal; ++i)
    {
        for (int j = 0; j < mapSize.columnVal; ++j)
        {
            int currentIndex = (i * mapSize.columnVal) + j;

            /* Water around the edges */
            if (i < 2 || i > 6)
                mapPtr[currentIndex] = &tileTypes[2];
            if (j < 2 || j > 13)
                mapPtr[currentIndex] = &tileTypes[2];

            /* Perimeter of brick wall */
            if (i > 1 && i < 7 && j > 1 && j < 14)
                mapPtr[currentIndex] = &tileTypes[1];

            /* Wood floor in the middle */
            if (i > 2 && i < 6 && j > 2 && j < 13)
                mapPtr[currentIndex] = &tileTypes[0];
        }
    }
}

const NW::Tile * const * NW::TileManager::GetTileMap() const
{
    return mapPtr.get();
}

const NW::Tile * NW::TileManager::GetTileType(int Index) const
{
    return &tileTypes[Index];
}

NW::RCVec NW::TileManager::GetMapSize()
{
    return mapSize;
}

NW::TileManager::TileManager(const GameManager& InGameManager) :
        gameManager(InGameManager)
        , mapSize{0, 0}
{
}
