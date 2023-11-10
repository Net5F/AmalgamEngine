#include "ItemData.h"
#include "Log.h"

namespace AM
{
namespace Client
{
ItemData::ItemData()
: ItemDataBase()
{
    // Add the null item.
    items.push_back(Item{"Null", "null", 0});
}

} // End namespace Client
} // End namespace AM
