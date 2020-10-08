#pragma once

namespace AM
{
/**
 * Represents an entity's position in the world.
 * When used for rendering a sprite, represents the top left.
 */
struct PositionComponent {
public:
    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** Current position. */
    float x = 0;
    float y = 0;
};

template<typename S>
void serialize(S& serializer, PositionComponent& positionComponent)
{
    serializer.value4b(positionComponent.x);
    serializer.value4b(positionComponent.y);
}

} // namespace AM
