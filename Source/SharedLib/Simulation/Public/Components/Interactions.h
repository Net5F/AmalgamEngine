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
 */
struct Interactions {
    /** The maximum number of interactions that an entity can support. */
    static constexpr std::size_t MAX_INTERACTIONS{4};

    /** The interactions that this entity supports. These show up in the 
        right-click list.
        The first interaction in this list will be used as the default 
        interaction.
        Note: This defaults to EngineInteractionType::NotSet. */
    std::array<Uint8, MAX_INTERACTIONS> supportedInteractions{};

    /**
     * Finds the first empty index in supportedInteractions and adds the given 
     * interaction.
     * If supportedInteractions is full, prints a warning and does nothing.
     */
    void addInteraction(Uint8 newInteraction)
    {
        for (Uint8& interaction : supportedInteractions) {
            if (interaction == EngineInteractionType::NotSet) {
                interaction = newInteraction;
                return;
            }
        }

        LOG_ERROR("Tried to add interaction to full array.");
    }
};

template<typename S>
void serialize(S& serializer, Interactions& interaction)
{
    serializer.container1b(interaction.supportedInteractions);
}

} // End namespace AM
