#include <ObjectManager.h>

//void NW::ObjectManager::AddObject(ObjectData InObjectData)
//{
//}

void NW::ObjectManager::RemoveObject(int InObjectID)
{
}

const NW::Object * const * NW::ObjectManager::GetObjectMap() const
{
    return mapPtr.get();
}

void NW::ObjectManager::SetMapSize(int InMapWidth, int InMapHeight)
{
    mapWidth = InMapWidth;
    mapHeight = InMapHeight;
}

NW::ObjectManager::ObjectManager(const GameManager& InGameManager
                                 , int InMapWidth, int InMapHeight) :
        gameManager(InGameManager)
        , mapWidth(InMapWidth)
        , mapHeight(InMapHeight)
{
}
