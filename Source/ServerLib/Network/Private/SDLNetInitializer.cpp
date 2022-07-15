#include "SDLNetInitializer.h"
#include "SDL_net.h"

namespace AM
{
namespace Server
{
SDLNetInitializer::SDLNetInitializer()
{
    SDLNet_Init();
}

SDLNetInitializer::~SDLNetInitializer()
{
    SDLNet_Quit();
}

} // end namespace Server
} // end namespace AM
