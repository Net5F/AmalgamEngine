#include "SDL_Wrappers/SDLNet.h"
#include "SDL_net.h"

namespace AM
{
SDLNet::SDLNet()
{
    SDLNet_Init();
}

SDLNet::~SDLNet()
{
    SDLNet_Quit();
}

} // end namespace AM
