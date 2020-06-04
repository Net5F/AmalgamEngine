#ifndef CLIENTCOMPONENT_H_
#define CLIENTCOMPONENT_H_

#include "NetworkDefs.h"

namespace AM
{
namespace Server
{

/**
 * Indicates that an entity is associated with a Client.
 * Tracks that Client's NetworkID so the Game knows which one to send
 * messages to.
 *
 * Also used to remove the entity from the game when the Client disconnects.
 */
struct ClientComponent
{
public:
    ClientComponent()
    : networkID(0)
    {
    }

    ClientComponent(NetworkID inNetworkID)
    : networkID(inNetworkID)
    {
    }

    NetworkID networkID;
};

} // namespace Server
} // namespace AM

#endif /* End CLIENTCOMPONENT_H_ */
