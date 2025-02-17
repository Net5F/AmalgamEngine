#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** An AudioVisualEffect's numeric ID.
    Note: AV effects aren't persisted, so this is purely a runtime ID used for 
          active effect instances. */
using AudioVisualEffectID = Uint16;

/**
 * The ID of the "null effect", or the ID used to indicate that an effect is
 * not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (avEffectID)".
 */
static constexpr AudioVisualEffectID NULL_AUDIO_VISUAL_EFFECT_ID{0};

} // End namespace AM
