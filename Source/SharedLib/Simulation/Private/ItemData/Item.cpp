#include "Item.h"

namespace AM
{
void Item::addInteraction(ItemInteractionType newInteraction)
{
    if ((newInteraction == ItemInteractionType::UseOn)
        || (newInteraction == ItemInteractionType::Destroy)
        || (newInteraction == ItemInteractionType::Examine)) {
        LOG_INFO(
            "Tried to add UseOn, Destroy, or Examine interaction (the client "
            "automatically adds these, no need to add them manually).");
        return;
    }

    for (ItemInteractionType& interaction : supportedInteractions) {
        if (interaction == newInteraction) {
            LOG_INFO("Tried to add already-present interaction: %u",
                      newInteraction);
        }
        else if (interaction == ItemInteractionType::NotSet) {
            interaction = newInteraction;
            return;
        }
    }

    LOG_INFO("Tried to add interaction to full array.");
}

bool Item::supportsInteraction(ItemInteractionType desiredInteraction) const
{
    // All items support UseOn, Destroy, and Examine.
    if ((desiredInteraction == ItemInteractionType::UseOn)
        || (desiredInteraction == ItemInteractionType::Destroy)
        || (desiredInteraction == ItemInteractionType::Examine)) {
        return true;
    }

    for (ItemInteractionType interaction : supportedInteractions) {
        if (interaction == desiredInteraction) {
            return true;
        }
    }

    return false;
}

std::array<ItemInteractionType, Item::MAX_INTERACTIONS>
    Item::getInteractionList() const
{
    std::array<ItemInteractionType, MAX_INTERACTIONS> interactionList{};

    // First, fill the list with this item's supported interactions.
    std::size_t nextIndex{0};
    for (ItemInteractionType interactionType : supportedInteractions) {
        if (interactionType != ItemInteractionType::NotSet) {
            interactionList[nextIndex++] = interactionType;
        }
        else {
            // No more interactions in supportedInteractions.
            break;
        }
    }

    // Then, fill it with UseOn, Destroy, and Examine.
    interactionList[nextIndex++] = ItemInteractionType::UseOn;
    interactionList[nextIndex++] = ItemInteractionType::Destroy;
    interactionList[nextIndex++] = ItemInteractionType::Examine;

    return interactionList;
}

ItemInteractionType Item::getDefaultInteraction() const
{
    // If this item doesn't have any custom interactions, return UseOn (the 
    // first built-in interaction).
    if (supportedInteractions[0] == ItemInteractionType::NotSet) {
        return ItemInteractionType::UseOn;
    }
    else {
        return supportedInteractions[0];
    }
}

std::size_t Item::getInteractionCount() const
{
    std::size_t interactionCount{0};
    for (ItemInteractionType interactionType : supportedInteractions) {
        if (interactionType != ItemInteractionType::NotSet) {
            interactionCount++;
        }
    }

    return interactionCount + BUILTIN_INTERACTION_COUNT;
}

} // namespace AM
