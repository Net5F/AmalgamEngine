#include "ItemData.h"
#include "Log.h"

namespace AM
{
namespace Server
{
ItemData::ItemData()
: ItemDataBase(true)
{
    // TODO: Load these from the database.
    // TEMP: Add some placeholder items.
    Item testItem{"Test1"};
    testItem.addProperty(Description{"A test item."});
    createItem(testItem);
}

} // End namespace Server
} // End namespace AM
