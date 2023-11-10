#include "ItemData.h"
#include "Log.h"

namespace AM
{
namespace Server
{
ItemData::ItemData()
: ItemDataBase()
{
    // Add the null item.
    items.push_back(Item{"Null", "null", 0});

    // TODO: Load these from the database.
    // TEMP: Add some placeholder items.
    items.push_back(Item{"Test1", "test1", 1});
}

} // End namespace Server
} // End namespace AM
