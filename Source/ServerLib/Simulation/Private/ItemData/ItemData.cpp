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
    createItem(Item{"Test1", "", NULL_ITEM_ID});
}

} // End namespace Server
} // End namespace AM
