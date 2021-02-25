#pragma once

#include "SDL_events.h"

namespace AM
{
namespace Client
{

/**
 * Simple interface to facilitate event handling and propagation.
 *
 * Specifically meant for OS events, provided by SDL2. Intra-app events have
 * other data flow paths.
 */
class EventHandler
{
public:
    virtual ~EventHandler() {};

    /**
     * Handles the given event.
     *
     * Since synchronous feedback is required, it's advised that an implementor
     * limit work done in this function. If significant time must be spent,
     * consider only doing as much work as is necessary to determine if the
     * event will be handled, and defer processing until later.
     *
     * @return true if the event has been handled and should stop propagating,
     *         else false.
     */
    virtual bool handleEvent(SDL_Event& event) = 0;
};

} // End namespace Client
} // End namespace AM
