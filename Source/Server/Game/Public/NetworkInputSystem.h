#ifndef NETWORKINPUTSYSTEM_H
#define NETWORKINPUTSYSTEM_H

namespace AM
{

class World;

class NetworkInputSystem
{
public:
    NetworkInputSystem(World& inWorld);

    /**
     * Processes incoming EntityUpdate messages.
     */
    void processInputEvents();

private:
    World& world;
};

} // namespace AM

#endif /* NETWORKINPUTSYSTEM_H */
