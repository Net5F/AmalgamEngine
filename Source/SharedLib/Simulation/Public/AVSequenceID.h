#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** An AVSequence's numeric ID.
    Note: AV sequences aren't persisted, so this is purely a runtime ID used for 
          active sequence instances. */
using AVSequenceID = Uint16;

/**
 * The ID of the "null sequence", or the ID used to indicate that an sequence is
 * not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (avSequenceID)".
 */
static constexpr AVSequenceID NULL_AV_SEQUENCE_ID{0};

} // End namespace AM
