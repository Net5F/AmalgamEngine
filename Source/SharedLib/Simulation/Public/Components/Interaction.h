#pragma once

#include "EngineInteractionType.h"
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
        Note: This defaults values to EngineInteractionType::NotSet. */
    std::array<Uint8, MAX_INTERACTIONS> supportedInteractions{};

    /**
     * Finds the first empty index in supportedInteractions and adds the given 
     * interaction.
     * If supportedInteractions is full, prints a warning and does nothing.
     */
    void add(Uint8 newInteraction)
    {
        for (Uint8& interaction : supportedInteractions) {
            if (interaction == EngineInteractionType::NotSet) {
                interaction = newInteraction;
                return;
            }
        }

        LOG_ERROR("Tried to add interaction to full array.");
    }

    /**
     * Returns true if this component contains the given interaction.
     */
    bool contains(Uint8 desiredInteraction)
    {
        for (Uint8 interaction : supportedInteractions) {
            if (interaction == desiredInteraction) {
                return true;
            }
        }

        return false;
    }
};

template<typename S>
void serialize(S& serializer, Interaction& interaction)
{
    serializer.container1b(interaction.supportedInteractions);
}

} // End namespace AM
