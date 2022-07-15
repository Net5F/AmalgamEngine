#pragma once

namespace AM
{
namespace Server
{
/**
 * Minimal helper class to facilitate calling SDLNet_Init from an initializer
 * list.
 *
 * SDLNet_Init must be called after SDL is initialized, but before any SDLNet
 * functions are called.
 */
class SDLNetInitializer
{
public:
    SDLNetInitializer();

    ~SDLNetInitializer();
};

} // End namespace Server
} // End namespace AM
