#pragma once

#include "boost/mp11/list.hpp"
#include <string>

namespace AM
{
/** The item's description. */
struct ItemDescription {
    /** The max length of an item's description text. */
    static constexpr std::size_t MAX_TEXT_LENGTH{500};

    std::string text{};
};
template<typename S>
void serialize(S& serializer, ItemDescription& itemDescription)
{
    serializer.text1b(itemDescription.text, ItemDescription::MAX_TEXT_LENGTH);
}

/** The list of engine properties that may be attached to an item. */
using EngineItemPropertyTypes = boost::mp11::mp_list<ItemDescription>;

} // End namespace AM
