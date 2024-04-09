#include "Item.h"
#include <algorithm>

namespace AM
{
bool Item::addInteraction(ItemInteractionType newInteraction)
{
    if (supportedInteractions.size()
        == SharedConfig::MAX_ENTITY_INTERACTIONS) {
        // The interaction limit has been reached.
        return false;
    }
    else if (std::ranges::find(supportedInteractions, newInteraction)
             != supportedInteractions.end()) {
        // The interaction is already present.
        return false;
    }

    supportedInteractions.emplace_back(newInteraction);
    return true;
}

bool Item::supportsInteraction(ItemInteractionType desiredInteraction) const
{
    // All items support UseOn, Destroy, and Examine.
    if ((desiredInteraction == ItemInteractionType::UseOn)
        || (desiredInteraction == ItemInteractionType::Destroy)
        || (desiredInteraction == ItemInteractionType::Examine)) {
        return true;
    }

    return (std::ranges::find(supportedInteractions, desiredInteraction)
            != supportedInteractions.end());
}

std::vector<ItemInteractionType> Item::getInteractionList() const
{
    // First, fill the list with this item's supported interactions.
    std::vector<ItemInteractionType> interactionList(supportedInteractions);

    // Then, fill it with UseOn, Destroy, and Examine.
    interactionList.emplace_back(ItemInteractionType::UseOn);
    interactionList.emplace_back(ItemInteractionType::Destroy);
    interactionList.emplace_back(ItemInteractionType::Examine);

    return interactionList;
}

ItemInteractionType Item::getDefaultInteraction() const
{
    // If this item doesn't have any custom interactions, return UseOn (the
    // first built-in interaction).
    if (supportedInteractions.empty()) {
        return ItemInteractionType::UseOn;
    }
    else {
        return supportedInteractions[0];
    }
}

std::size_t Item::getInteractionCount() const
{
    return supportedInteractions.size() + BUILTIN_INTERACTION_COUNT;
}

} // namespace AM
