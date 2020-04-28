#ifndef MESSAGEUTIL_H
#define MESSAGEUTIL_H

#include "InputComponent.h"
#include "Message_generated.h"

namespace AM
{

/**
 * Shared static messaging utility functions.
 */
class MessageUtil {
public:
    /**
     * Converts AM input states to the flatbuffer equivalent.
     */
    static fb::InputState convertToFbInputState(Input::State state);


    /**
     * Converts flatbuffer input states to the AM equivalent.
     */
    static Input::State convertToAMInputState(fb::InputState state);
};

} // namespace AM

#endif /* MESSAGEUTIL_H */
