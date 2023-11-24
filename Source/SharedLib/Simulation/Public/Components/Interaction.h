#pragma once

#include "EntityInteractionType.h"
#include "Log.h"
#include <SDL_stdinc.h>
#include <array>

namespace AM
{

/**
 * Represents the interactions that an entity supports, i.e. the options that 
 * show up when you right click them.
 *
 * Note: This component won't be present on an entity if it has no interactions.
 */
struct Interaction {
    /** The maximum number of interactions that an entity can support. */
    static constexpr std::size_t MAX_INTERACTIONS{4};

    /** The interactions that this entity supports.
        Elements are filled contiguously starting at index 0. Empty elements 
        will be at the end.
        The first interaction in this list is the default interaction.
        Note: This defaults values to EntityInteractionType::NotSet. */
    std::array<EntityInteractionType, MAX_INTERACTIONS> supportedInteractions{};

    /**
     * Finds the first empty index in supportedInteractions and adds the given 
     * interaction.
     * If supportedInteractions is full, prints a warning and does nothing.
     */
    void add(EntityInteractionType newInteraction)
    {
        for (EntityInteractionType& interaction : supportedInteractions) {
            if (interaction == newInteraction) {
                LOG_ERROR("Tried to add already-present interaction: %u",
                          newInteraction);
            }
            else if (interaction == EntityInteractionType::NotSet) {
                interaction = newInteraction;
                return;
            }
        }

        LOG_ERROR("Tried to add interaction to full array.");
    }

    /**
     * Returns true if this component supports the given interaction.
     */
    bool supports(EntityInteractionType desiredInteraction) const
    {
        for (EntityInteractionType interaction : supportedInteractions) {
            if (interaction == desiredInteraction) {
                return true;
            }
        }

        return false;
    }

    /**
     * Returns true if this component doesn't contain any interactions.
     */
    bool isEmpty() const
    {
        return (supportedInteractions[0] == EntityInteractionType::NotSet);
    }

    /**
     * Returns this component's default interaction.
     */
    EntityInteractionType getDefault() const
    {
        return supportedInteractions[0];
    }

    /**
     * Returns the number of interactions that this component supports.
     */
    std::size_t getCount() const
    {
        std::size_t interactionCount{0};
        for (EntityInteractionType interactionType : supportedInteractions) {
            if (interactionType != EntityInteractionType::NotSet) {
                interactionCount++;
            }
        }

        return interactionCount;
    }
};

template<typename S>
void serialize(S& serializer, Interaction& interaction)
{
    serializer.container1b(interaction.supportedInteractions);
}

} // End namespace AM
