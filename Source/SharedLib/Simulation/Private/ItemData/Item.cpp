#include "Item.h"

namespace AM
{
void Item::addInteraction(ItemInteractionType newInteraction)
{
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

bool Item::containsInteraction(ItemInteractionType desiredInteraction) const
{
    // All items support Examine.
    if (desiredInteraction == ItemInteractionType::Examine) {
        return true;
    }

    for (ItemInteractionType interaction : supportedInteractions) {
        if (interaction == desiredInteraction) {
            return true;
        }
    }

    return false;
}

} // namespace AM
