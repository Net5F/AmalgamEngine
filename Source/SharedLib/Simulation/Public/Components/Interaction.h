#pragma once

#include "EntityInteractionType.h"
#include "SharedConfig.h"
#include <vector>

namespace AM
{

/**
 * Represents the interactions that an entity supports, i.e. the options that
 * show up when you right click them.
 *
 * Note: This component won't be present on an entity if it has no interactions.
 */
struct Interaction {
    /** The interactions that this entity supports.
        The first interaction in this list is the default interaction. */
    std::vector<EntityInteractionType> supportedInteractions{};

    /**
     * Adds the given interaction to supportedInteractions.
     * @return true if the interaction was added, else false (already present 
     *         or array was full).
     */
    bool add(EntityInteractionType newInteraction);

    /**
     * Returns true if this component supports the given interaction.
     */
    bool supports(EntityInteractionType desiredInteraction) const;
};

template<typename S>
void serialize(S& serializer, Interaction& interaction)
{
    serializer.container1b(interaction.supportedInteractions,
                           SharedConfig::MAX_ENTITY_INTERACTIONS);
}

} // End namespace AM
