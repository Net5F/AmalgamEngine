#include "SDL_Wrappers/SDLNet.h"
#include <SDL3_net/SDL_net.h>

namespace AM
{
SDLNet::SDLNet()
{
    NET_Init();
}

SDLNet::~SDLNet()
{
    NET_Quit();
}

} // end namespace AM
