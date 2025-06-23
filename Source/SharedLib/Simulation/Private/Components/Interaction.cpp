#include "Interaction.h"
#include "SharedConfig.h"
#include "Log.h"
#include "AMAssert.h"
#include <algorithm>

namespace AM
{

bool Interaction::add(EntityInteractionType newInteraction)
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

bool Interaction::supports(EntityInteractionType desiredInteraction) const
{
    return (std::ranges::find(supportedInteractions, desiredInteraction)
            != supportedInteractions.end());
}

} // End namespace AM
