#pragma once

namespace AM
{
namespace Client
{
/**
 * Represents a 2D rectangle in isometric screen space.
 * Fields are float to maintain precision. Cast right before rendering.
 */
struct ScreenRect {
public:
    float x{0};
    float y{0};
    float width{0};
    float height{0};
};

} // namespace Client
} // namespace AM
