#pragma once

namespace AM
{
/**
 * RAII wrapper for SDL_Net.
 *
 * SDLNet_Init must be called after SDL is initialized, but before any SDLNet
 * functions are called.
 */
class SDLNet
{
public:
    SDLNet();

    ~SDLNet();
};

} // End namespace AM
