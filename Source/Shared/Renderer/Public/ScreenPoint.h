#pragma once

namespace AM
{
/**
 * Represents a 2D point in isometric screen space.
 * Fields are float to maintain precision. Cast right before rendering.
 */
struct ScreenPoint {
public:
    float x{0};
    float y{0};
};

} // namespace AM
