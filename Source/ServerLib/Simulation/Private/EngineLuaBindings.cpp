#include "EngineLuaBindings.h"
#include "World.h"
#include "sol/sol.hpp"

namespace AM
{
namespace Server
{

EngineLuaBindings::EngineLuaBindings(sol::state& inEntityInitLua,
                                     sol::state& inItemInitLua, World& inWorld)
: entityInitLua{inEntityInitLua}
, itemInitLua{inItemInitLua}
, world{inWorld}
{
}

void EngineLuaBindings::addBindings()
{
    // Entity init

    // Item init
    itemInitLua.set_function("addDescription",
                             &EngineLuaBindings::addDescription,
                             this);
    itemInitLua.set_function("addCombination",
                             &EngineLuaBindings::addCombination,
                             this);
}

void EngineLuaBindings::addDescription(const std::string& description)
{
    // All items support the Examine interaction, so we only need to add the 
    // description property.
    Item* item{itemInitLua.get<Item*>("selfItemPtr")};
    item->properties.push_back(ItemDescription{description});
}

void EngineLuaBindings::addCombination(const std::string& otherItemID,
                                       const std::string& resultItemID,
                                       const std::string& description)
{
    const Item* otherItem{world.itemData.getItem(otherItemID)};
    const Item* resultItem{world.itemData.getItem(resultItemID)};

    // Try to add the given combination.
    if (otherItem && resultItem) {
        Item* item{itemInitLua.get<Item*>("selfItemPtr")};
        item->itemCombinations.emplace_back(otherItem->numericID,
                                            resultItem->numericID, description);
    }
    else {
        std::string errorString{
            "Failed to add item combination. Item(s) not found: "};
        if (!otherItem) {
            errorString += "\"" + otherItemID + "\" ";
        }
        if (!resultItem) {
            errorString += "\"" + resultItemID + "\" ";
        }

        throw std::runtime_error{errorString};
    }
}

} // namespace Server
} // namespace AM
