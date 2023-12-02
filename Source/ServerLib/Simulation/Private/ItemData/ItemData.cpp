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
    Item testItem{.displayName{"Test1"}, .iconID{1}};
    testItem.addProperty(ItemDescription{"A test item."});
    createItem(testItem);

    Item testItem2{.displayName{"Test2"}, .iconID{2}};
    testItem2.addProperty(ItemDescription{"Another test item."});
    testItem2.itemCombinations.emplace_back(
        1, 3, "The items combine into a NEW ITEM.");
    createItem(testItem2);

    Item testItem3{.displayName{"Combined Item"}, .iconID{3}};
    testItem3.addProperty(ItemDescription{"A combined item."});
    createItem(testItem3);
}

} // End namespace Server
} // End namespace AM
