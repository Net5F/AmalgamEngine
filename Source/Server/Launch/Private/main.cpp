#include <SDL2pp/SDL2pp.hh>
#include "messend.hpp"
#include "Message_generated.h"

#include "SharedDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"

#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <queue>
#include <algorithm>
#include <atomic>
#include <thread>

namespace AM
{
static constexpr Uint32 SCREEN_WIDTH = 1280;

static constexpr Uint32 SCREEN_HEIGHT = 720;
}

using namespace AM;

class IDPool
{
public:
    static uint32_t reserveID()
    {
        for (uint16_t i = 0; i < MAX_ENTITIES; ++i) {
            // Find the first false.
            if (!(IDs[i])) {
                IDs[i] = true;
                return i;
            }
        }

        std::cerr << "Tried to reserve ID when all were taken. Returning 0." << std::endl;
        return 0;
    }

    static void freeID(uint32_t ID)
    {
        if (IDs[ID]) {
            IDs[ID] = false;
        }
        else {
            std::cerr << "Tried to free an unused ID." << std::endl;
        }
    }

private:
    /**
     * If ID 'x' is available, IDs[x] will be true. Else, it will be false.
     */
    static std::array<bool, MAX_ENTITIES> IDs;
};
std::array<bool, MAX_ENTITIES> IDPool::IDs = {}; // Init to 0;

class World
{
public:
    World()
    : entityNames {}, positions {}, movements {}, inputs {}, sprites {}, componentFlags {}
    {
    }

    EntityID AddEntity(const std::string& name)
    {
        EntityID id = IDPool::reserveID();
        entityNames[id] = name;

        return id;
    }

    void RemoveEntity(EntityID entityID)
    {
        componentFlags[entityID] = 0;
        entityNames[entityID] = "";
    }

    /**
     * Registers this entity as possessing this component.
     * The caller is in charge of making sure the state of the component is appropriate.
     */
    void AttachComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
    {
        // If the entity doesn't have the component, add it.
        if ((componentFlags[entityID] & componentFlag) == 0) {
            componentFlags[entityID] |= componentFlag;
        }
        else {
            std::cerr << "Tried to add component when entity already has it."
            << std::endl;
        }
    }

    void RemoveComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
    {
        // If the entity has the component, remove it.
        if ((componentFlags[entityID] & componentFlag) == componentFlag) {
            componentFlags[entityID] |= componentFlag;
        }
        else {
            std::cerr << "Tried to remove component when entity doesn't have it."
            << std::endl;
        }
    }

    /** Entity data lists. */
    std::array<std::string, MAX_ENTITIES> entityNames;
    std::array<PositionComponent, MAX_ENTITIES> positions;
    std::array<MovementComponent, MAX_ENTITIES> movements;
    std::array<InputComponent, MAX_ENTITIES> inputs;
    std::array<SpriteComponent, MAX_ENTITIES> sprites;
    // Bit flags for every component, indicating whether the object at a given index has that component.
    std::array<uint32_t, MAX_ENTITIES> componentFlags;
};

class NetworkInputSystem
{
public:
    NetworkInputSystem(World& inWorld)
    : world(inWorld)
    {
    }

    /**
     * Processes incoming EntityUpdate messages.
     */
    void processInputEvents()
    {
    }

private:
    World& world;
};

class MovementSystem
{
public:
    MovementSystem(World& inWorld)
    : world(inWorld)
    {
    }

    /**
     * Updates movement components based on input state, moves position components based on movement, updates sprites based on position.
     */
    void processMovements()
    {
        for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
            /* Process input state on everything that has an input component and a movement component. */
            if ((world.componentFlags[entityID] & ComponentFlag::Input)
            && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
                // Process the input state for each entity.
                changeVelocity(entityID, world.inputs[entityID].inputStates);
            }

            /* Move all entities that have a position and movement component. */
            if ((world.componentFlags[entityID] & ComponentFlag::Position)
            && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
                // Update the positions based on the velocities.
                world.positions[entityID].x += world.movements[entityID].velX;
                world.positions[entityID].y += world.movements[entityID].velY;
            }

            /* Move the sprites to the new positions. */
            if ((world.componentFlags[entityID] & ComponentFlag::Position)
            && (world.componentFlags[entityID] & ComponentFlag::Sprite)) {
                world.sprites[entityID].posInWorld.x = world.positions[entityID].x;
                world.sprites[entityID].posInWorld.y = world.positions[entityID].y;
            }
        }
    }

private:
    void changeVelocity(
    EntityID entityID,
    std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates)
    {
        MovementComponent& movement = world.movements[entityID];
        // Handle up/down (favors up).
        if (inputStates[Input::Up] == Input::Pressed) {
            movement.velY -= 0.25;

            if (movement.velY < movement.maxVelY) {
                movement.velY = -(movement.maxVelY);
            }
        }
        else if (inputStates[Input::Down] == Input::Pressed) {
            movement.velY += 0.25;

            if (movement.velY > movement.maxVelY) {
                movement.velY = movement.maxVelY;
            }
        }
        else {
            // Slow the entity down.
            if (movement.velY > 0) {
                movement.velY -= 0.25;
            }
            else if (movement.velY < 0) {
                movement.velY += 0.25;
            }
        }

        // Handle left/right (favors right).
        if (inputStates[Input::Left] == Input::Pressed) {
            movement.velX -= 0.25;

            if (movement.velX < movement.maxVelX) {
                movement.velX = -(movement.maxVelX);
            }
        }
        else if (inputStates[Input::Right] == Input::Pressed) {
            movement.velX += 0.25;

            if (movement.velX > movement.maxVelX) {
                movement.velX = movement.maxVelX;
            }
        }
        else {
            // Slow the entity down.
            if (movement.velX > 0) {
                movement.velX -= 0.25;
            }
            else if (movement.velX < 0) {
                movement.velX += 0.25;
            }
        }
    }

    World& world;
};

class RenderSystem
{
public:
    RenderSystem(World& inWorld)
    : world(inWorld)
    {
    }

    void collectRenderObjects()
    {
    }

    void render()
    {
    }

private:
    World& world;
};

int main(int argc, char **argv)
try
{
    // Set up the SDL constructs.
    // TODO: Check if constructing this is necessary for for networking.
    SDL2pp::SDL sdl();

    // Calc the center of the screen.
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;

    // Set up our world.
    World world;

    // Set up our systems.
    NetworkInputSystem networkInputSystem(world);
    MovementSystem movementSystem(world);

    // Set up the networking.
    msnd::startup();
    msnd::Acceptor acceptor("127.0.0.1", 41499);
    std::vector<std::unique_ptr<msnd::Peer>> clients;

    // Spin up a thread to check for command line input.
    std::atomic<bool> bQuit = false;
    std::thread([&]
    {
        while (1) {
            std::string userInput = "";
            std::getline(std::cin, userInput);
            if (userInput == "exit") {
                bQuit = true;
            }
        }
    }).detach();

    std::cout << "Starting main loop." << std::endl;
    while (!bQuit) {
        // Check for new connections.
        std::unique_ptr<msnd::Peer> newClient = acceptor.accept();
        if (newClient != nullptr) {
            std::cout << "New peer connected." << std::endl;
            clients.push_back(std::move(newClient));

            // Build their entity.

            // Send them their ID.
        }

        // Check for disconnects.
        for (auto i = clients.begin(); i != clients.end(); ++i) {
            if (!((*i)->isConnected())) {
                clients.erase(i);
            }
        }

        // Will return Input::Type::Exit if the app needs to exit.
        networkInputSystem.processInputEvents();

        movementSystem.processMovements();

        SDL_Delay(1);
    }

    return 0;
}
catch (SDL2pp::Exception& e) {
    std::cerr << "Error in: " << e.GetSDLFunction() << std::endl;
    std::cerr << "  Reason:  " << e.GetSDLError() << std::endl;
    return 1;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
