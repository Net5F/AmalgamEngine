#pragma once

namespace AM
{
/**
 * Represents the top left point of an entity's position in the world.
 */
struct Position {
public:
    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** Current position. */
    float x{0};
    float y{0};
    float z{0};

    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    /** Previous position, optional, used for lerping. */
    float oldX{0};
    float oldY{0};
    float oldZ{0};
};

template<typename S>
void serialize(S& serializer, Position& position)
{
    serializer.value4b(position.x);
    serializer.value4b(position.y);
    serializer.value4b(position.z);
}

} // namespace AM
