#pragma once

namespace AM
{
namespace Client
{
/**
 * Minimal helper class to facilitate calling UserConfig::get() from an 
 * initializer list.
 *
 * UserConfig::get() must first be called after SDL is initialized, but before 
 * any threads are spun up that may cause race conditions around UserConfig 
 * member access.
 */
class UserConfigInitializer
{
public:
    UserConfigInitializer();
};

} // End namespace Client
} // End namespace AM
