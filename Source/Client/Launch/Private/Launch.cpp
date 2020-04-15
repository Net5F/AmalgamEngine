//#include <Message_generated.h>
//#include <messend.hpp>
//#include <iostream>
//#include <memory>
//#include <stdio.h>
#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <queue>
#include <algorithm>
#include <forward_list>
#include <SDL2pp/SDL2pp.hh>

// Anonymous namespace restricts vars to this translation unit
namespace
{
static constexpr uint32_t SCREEN_WIDTH = 1280;

static constexpr uint32_t SCREEN_HEIGHT = 720;

static constexpr uint32_t MAX_ENTITIES = 100;
}

struct PositionComponent
{
public:
    PositionComponent()
    : x(0)
    , y(0)
    {
    }

    /** Current position. */
    float x;
    float y;
};

struct MovementComponent
{
public:
    MovementComponent()
    : velX(0)
    , velY(0)
    , maxVelY(5)
    , maxVelX(5)
    {
    }

    /** Current velocities. */
    float velX;
    float velY;
    float maxVelX;
    float maxVelY;
};

enum class Input
{
    None,
    Up,
    Down,
    Left,
    Right,
    Exit,// Exit the application.
    NumInputs
};

struct InputComponent
{
public:
    /** The last received input. */
    std::queue<Input> inputQueue;
};

struct SpriteComponent
{
public:
    SpriteComponent()
    : texturePtr(nullptr)
    , posInTexture{0, 0, 0, 0}
    , posInWorld{0, 0, 0, 0}
    {
    }

    /** A pointer to the texture that holds this sprite. */
    std::shared_ptr<SDL2pp::Texture> texturePtr;

    /** UV position and size in texture. */
    SDL2pp::Rect posInTexture;

    /** ST position and size in world. */
    SDL2pp::Rect posInWorld;
};

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

typedef uint32_t EntityID;

struct ComponentFlag {
    enum FlagType
    {
        Position = 1 << 0,
        Movement = 1 << 1,
        Input = 1 << 2,
        Sprite = 1 << 3
    };
};

class World
{
public:
    World()
    : entityNames{}
    , positions{}
    , movements{}
    , inputs{}
    , sprites{}
    , componentFlags{}
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
            std::cerr << "Tried to add component when entity already has it." << std::endl;
        }
    }

    void RemoveComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
    {
        // If the entity has the component, remove it.
        if ((componentFlags[entityID] & componentFlag) == componentFlag) {
            componentFlags[entityID] |= componentFlag;
        }
        else {
            std::cerr << "Tried to remove component when entity doesn't have it." << std::endl;
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

class InputSystem
{
public:
    InputSystem(World& inWorld)
    : world(inWorld)
    {
    }

    /**
     * Processes the waiting input events.
     *
     * If an input relevant to the components is received, modifies them appropriately.
     * If an input relevant to main is received, returns it.
     *
     * @return Input::None if nothing relevant to main was received, else returns the relevant input.
     */
    Input processInputEvents()
    {
        // Process all events.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                return Input::Exit;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                Input keyInput = Input::None;
                switch (event.key.keysym.sym)
                {
                    case SDLK_w:
                        keyInput = Input::Up;
                        break;
                    case SDLK_a:
                        keyInput = Input::Left;
                        break;
                    case SDLK_s:
                        keyInput = Input::Down;
                        break;
                    case SDLK_d:
                        keyInput = Input::Right;
                        break;
                    case SDLK_ESCAPE:
                        return Input::Exit;
                }

                // Push the input to all InputComponents.
                for (size_t i = 0; i < MAX_ENTITIES; ++i) {
                    if (world.componentFlags[i] & ComponentFlag::Input) {
                        world.inputs[i].inputQueue.push(keyInput);
                    }
                }
            }
        }

        return Input::None;
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

    void processMovements()
    {
        for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
            /* Process inputs on everything that has an input component and a movement component. */
            if ((world.componentFlags[entityID] & ComponentFlag::Input)
                    && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
                std::queue<Input>& inputQueue = world.inputs[entityID].inputQueue;
                if (inputQueue.empty()) {
                    // If we have no inputs, process the velocity appropriately.
                    changeVelocity(entityID, Input::None);
                }
                else {
                    // Process all the inputs in the component's queue.
                    while (!(inputQueue.empty())) {
                        changeVelocity(entityID, inputQueue.front());
                        inputQueue.pop();
                    }
                }
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
    void changeVelocity(EntityID entityID, Input input) {
        MovementComponent& movement = world.movements[entityID];
        switch (input) {
            case Input::Up:
                movement.velY -= 0.25;
                std::cout << "Input: Up. velY: " << movement.velY << std::endl;
                break;
            case Input::Left:
                movement.velX -= 0.25;
                break;
            case Input::Down:
                movement.velY += 0.25;
                break;
            case Input::Right:
                movement.velX += 0.25;
                break;
            case Input::None:
                std::cout << "Input: None." << std::endl;
                // Slow the entity down.
                if (movement.velX > 0) {
                    movement.velX -= 0.25;
                }
                else if (movement.velX < 0) {
                    movement.velX += 0.25;
                }

                if (movement.velY > 0) {
                    movement.velY -= 0.25;
                }
                else if (movement.velY < 0) {
                    movement.velY += 0.25;
                }
                break;
        }

        // Lock movement to the max velocity.
        if (movement.velX > movement.maxVelX) {
            std::cout << "Hit max" << std::endl;
            movement.velX = movement.maxVelX;
        }
        if (movement.velY > movement.maxVelY) {
            std::cout << "Hit max" << std::endl;
            movement.velY = movement.maxVelY;
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

int main(int argc, char **argv) try
{
    // Setup the SDL constructs.
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Amalgam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    std::shared_ptr<SDL2pp::Texture> sprites = std::make_shared<SDL2pp::Texture>(renderer, "Resources/u4_tiles_pc_ega.png");

    // Calc the center of the screen.
    int centerX = renderer.GetOutputWidth() / 2;
    int centerY = renderer.GetOutputHeight() / 2;

    // Setup our world.
    World world;

    // Setup our systems.
    InputSystem inputSystem(world);
    MovementSystem movementSystem(world);
    RenderSystem renderSystem(world);

    // Setup our player.
    SDL2pp::Rect textureRect(0, 32, 16, 16);
    SDL2pp::Rect worldRect(centerX - 64, centerY - 64, 64, 64);

    EntityID player = world.AddEntity("Player");
    world.positions[player].x = centerX - 64;
    world.positions[player].y = centerY - 64;
    world.movements[player].maxVelX = 5;
    world.movements[player].maxVelY = 5;
    world.sprites[player].texturePtr = sprites;
    world.sprites[player].posInTexture = textureRect;
    world.sprites[player].posInWorld = worldRect;
    world.AttachComponent(player, ComponentFlag::Input);
    world.AttachComponent(player, ComponentFlag::Movement);
    world.AttachComponent(player, ComponentFlag::Position);
    world.AttachComponent(player, ComponentFlag::Sprite);

    bool bQuit = false;
    while (!bQuit)
    {
        // Will return Input::Exit if the app needs to exit.
        Input input = inputSystem.processInputEvents();
        if (input == Input::Exit) {
            break;
        }

        movementSystem.processMovements();

        renderer.Clear();

        renderer.Copy(*(world.sprites[player].texturePtr)
                      , world.sprites[player].posInTexture
                      , world.sprites[player].posInWorld);

        renderer.Present();

        SDL_Delay(1);
    }
    /*
    flatbuffers::FlatBufferBuilder builder;

    msnd::startup();

    std::unique_ptr<msnd::Peer> server = msnd::initiate("127.0.0.1", 41499);

    if (server == nullptr)
    {
        return 1;
    }

    while (1)
    {
        if (!(server->isConnected()))
        {
            std::cout << "Disconnected.\n";
            break;
        }
        else
        {
            std::unique_ptr<msnd::Message> response
                    = server->receiveMessage();
            if (response != nullptr)
            {
                auto message = NW::GetMessage(response->data);

                printf("Type: %d, Pos: (%d, %d)\n", message->type()
                       , message->pos()->row(), message->pos()->column());
            }
        }
    }

    msnd::shutdown();
    */

    return 0;
}
catch (SDL2pp::Exception& e)
{
    std::cerr << "Error in: " << e.GetSDLFunction() << std::endl;
    std::cerr << "  Reason:  " << e.GetSDLError() << std::endl;
    return 1;
}
catch (std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
