#pragma once

namespace AM
{
/**
 * Represents the current velocity of an entity.
 */
struct Velocity {
    static constexpr float DEFAULT_MAX_VELOCITY{5};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    float x{0};
    float y{0};
    float z{0};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    float maxX{DEFAULT_MAX_VELOCITY};
    float maxY{DEFAULT_MAX_VELOCITY};
    float maxZ{DEFAULT_MAX_VELOCITY};
};

template<typename S>
void serialize(S& serializer, Velocity& velocity)
{
    serializer.value4b(velocity.x);
    serializer.value4b(velocity.y);
    serializer.value4b(velocity.z);
}

} // namespace AM
