#include <SDL2pp/SDL2pp.hh>
#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <queue>
#include <algorithm>
#include <messend.hpp>
#include <Message_generated.h>

// Anonymous namespace restricts vars to this translation unit
namespace
{
static constexpr uint32_t SCREEN_WIDTH = 1280;

static constexpr uint32_t SCREEN_HEIGHT = 720;

static constexpr uint32_t MAX_ENTITIES = 100;
}

using namespace AM;

class Network
{
public:
    Network()
    : server(nullptr)
    {
        msnd::startup();
    }

    ~Network()
    {
        msnd::shutdown();
    }

    bool connect()
    {
        // Try to connect.
        server = msnd::initiate("127.0.0.1", 41499);
        return (server != nullptr) ? true : false;
    }

    void send(msnd::Message messageToSend)
    {
        if (!(server->isConnected())) {
            std::cerr << "Tried to send while server is disconnected." << std::endl;
            return;
        }

        server->sendMessage(messageToSend);
    }

    std::unique_ptr<msnd::Message> receive()
    {
        return server->receiveMessage();
    }

private:
    std::unique_ptr<msnd::Peer> server;
};

struct PositionComponent
{
public:
    PositionComponent()
    : x(0), y(0)
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
    : velX(0), velY(0), maxVelY(5), maxVelX(5)
    {
    }

    /** Current velocities. */
    float velX;
    float velY;
    float maxVelX;
    float maxVelY;
};

struct Input
{
    enum Type
    {
        None,
        Up,
        Down,
        Left,
        Right,
        Exit, // Exit the application.
        NumTypes
    };

    enum State
    {
        Invalid,
        Pressed,
        Released
    };

    Input(Type inType, State inState)
    : type(inType), state(inState)
    {
    }

    Type type;
    State state;
};

struct InputComponent
{
public:
    InputComponent()
    {
        inputStates.fill(Input::Released);
    }

    /** Holds the current state of the inputs. */
    std::array<Input::State, Input::NumTypes> inputStates;
};

struct SpriteComponent
{
public:
    SpriteComponent()
    : texturePtr(nullptr), posInTexture { 0, 0, 0, 0 }, posInWorld { 0, 0, 0, 0 }
    {
    }

    // TODO: Switch to textureID and add a texture loader.
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

struct ComponentFlag
{
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
    : entityNames {},
      positions {},
      movements {},
      inputs {},
      sprites {},
      componentFlags {},
      playerID(0)
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

    /**
     * Registers an entity as being the player. Various systems will only apply to this entity.
     */
    void registerPlayerID(EntityID inPlayerID)
    {
        playerID = inPlayerID;
    }

    EntityID getPlayerID()
    {
        return playerID;
    }

    /** Entity data lists. */
    std::array<std::string, MAX_ENTITIES> entityNames;
    std::array<PositionComponent, MAX_ENTITIES> positions;
    std::array<MovementComponent, MAX_ENTITIES> movements;
    std::array<InputComponent, MAX_ENTITIES> inputs;
    std::array<SpriteComponent, MAX_ENTITIES> sprites;
    // Bit flags for every component, indicating whether the object at a given index has that component.
    std::array<uint32_t, MAX_ENTITIES> componentFlags;

    EntityID playerID;
};

class PlayerInputSystem
{
public:
    PlayerInputSystem(World& inWorld, Network& inNetwork)
    : world(inWorld)
    , network(inNetwork)
    , builder(512)
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
        bool stateChanged = false;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return {Input::Exit, Input::Pressed};
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                Input::State inputType =
                (event.type == SDL_KEYDOWN) ? Input::Pressed : Input::Released;
                Input keyInput = { Input::None, inputType };

                switch (event.key.keysym.sym)
                {
                    case SDLK_w:
                        keyInput.type = Input::Up;
                        break;
                    case SDLK_a:
                        keyInput.type = Input::Left;
                        break;
                    case SDLK_s:
                        keyInput.type = Input::Down;
                        break;
                    case SDLK_d:
                        keyInput.type = Input::Right;
                        break;
                    case SDLK_ESCAPE:
                        return {Input::Exit, Input::Pressed};
                }

                // Push the input to the player's InputComponent.
                EntityID player = world.getPlayerID();
                Input::State& entityState = world.inputs[player].inputStates[keyInput.type];

                // Only update on change.
                if (entityState != keyInput.state) {
                    stateChanged = true;
                    entityState = keyInput.state;
                }
            }
        }

        // If a change occurred, send the updated player input state to the server.
        if (stateChanged) {
            sendInputState();
        }

        return {Input::None, Input::Invalid};
    }

    void sendInputState()
    {
        builder.Clear();
        EntityID playerID = world.getPlayerID();

        // Translate the inputs to fb's enum.
        fb::InputState fbInputStates[Input::Type::NumTypes];
        for (uint8_t i = 0; i < Input::Type::NumTypes; ++i) {
            // Translate the Input::State enum to fb::InputState.
            fbInputStates[i] = convertToFbInputState(
                world.inputs[playerID].inputStates[i]);
        }
        flatbuffers::Offset<flatbuffers::Vector<fb::InputState>> inputVector =
            builder.CreateVector(fbInputStates, Input::Type::NumTypes);

        // Build the inputComponent.
        flatbuffers::Offset<fb::InputComponent> inputComponent = fb::CreateInputComponent(
            builder, inputVector);

        // Build the Entity.
        auto entityName = builder.CreateString(world.entityNames[playerID]);
        fb::EntityBuilder entityBuilder(builder);
        entityBuilder.add_id(playerID);
        entityBuilder.add_name(entityName);

        // Mark that we only are sending the InputComponent.
        entityBuilder.add_flags(ComponentFlag::Input);
        entityBuilder.add_inputComponent(inputComponent);

        std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
        entityVector.push_back(entityBuilder.Finish());
        auto entity = builder.CreateVector(entityVector);

        // Build an EntityUpdate.
        flatbuffers::Offset<fb::EntityUpdate> entityUpdate = fb::CreateEntityUpdate(
            builder, entity);

        // Build a Message.
        fb::MessageBuilder messageBuilder(builder);
        messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
        messageBuilder.add_content(entityUpdate.Union());
        flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();
        builder.Finish(message);

//        network.send(msnd::Message(builder.GetBufferPointer(), builder.GetSize()));

        /** Temporary: Try to read from the buffer. */
        const fb::Message* readMessage = fb::GetMessage(builder.GetBufferPointer());
        const fb::EntityUpdate* readEntityUpdate =
            static_cast<const fb::EntityUpdate*>(readMessage->content());
        auto readEntity = readEntityUpdate->entities()->Get(0);

        std::cout << "Entity: " << readEntity->name()->c_str() << std::endl;
        std::cout << "ID: " << readEntity->id() << std::endl;
        std::cout << "Flags: " << std::hex << readEntity->flags() << std::endl;
        if (readEntity->inputComponent()->inputStates()->Get(4) == fb::InputState::Pressed) {
            std::cout << "Right pressed" << std::endl;
        }
        else if (readEntity->inputComponent()->inputStates()->Get(4) == fb::InputState::Released) {
            std::cout << "Right released" << std::endl;
        }

//        while (1) {
//            if (!(server->isConnected())) {
//                std::cout << "Disconnected.\n";
//                break;
//            }
//            else {
//                std::unique_ptr<msnd::Message> response = server->receiveMessage();
//                if (response != nullptr) {
//                    Message message = GetMessage(response->data);
//
//                    switch (message.content_type()) {
//                        case MessageContent::MessageContent_EntityUpdate
//                    }
//                    printf("Type: %d, Pos: (%d, %d)\n", message.,
//                           message->pos()->row(), message->pos()->column());
//                }
//            }
//        }
    }

private:
    fb::InputState convertToFbInputState(Input::State state) {
        switch (state) {
            case Input::Invalid:
                return fb::InputState::Invalid;
                break;
            case Input::Pressed:
                return fb::InputState::Pressed;
                break;
            case Input::Released:
                return fb::InputState::Released;
                break;
            default:
                return fb::InputState::Invalid;
        }
    }

    World& world;
    Network& network;
    flatbuffers::FlatBufferBuilder builder;
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
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Amalgam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    std::shared_ptr<SDL2pp::Texture> sprites = std::make_shared<SDL2pp::Texture>(
    renderer, "Resources/u4_tiles_pc_ega.png");

    // Calc the center of the screen.
    int centerX = renderer.GetOutputWidth() / 2;
    int centerY = renderer.GetOutputHeight() / 2;

    // Set up our world.
    World world;

    // Connect to the server.
    Network network;
    while (!(network.connect())) {
        std::cerr << "Network failed to connect. Retrying." << std::endl;
    }

    // Set up our systems.
    PlayerInputSystem playerInputSystem(world, network);
    MovementSystem movementSystem(world);
    RenderSystem renderSystem(world);

    // Set up our player.
    SDL2pp::Rect textureRect(0, 32, 16, 16);
    SDL2pp::Rect worldRect(centerX - 64, centerY - 64, 64, 64);

    EntityID player = world.AddEntity("Player");
    world.positions[player].x = centerX - 64;
    world.positions[player].y = centerY - 64;
    world.movements[player].maxVelX = 1;
    world.movements[player].maxVelY = 1;
    world.sprites[player].texturePtr = sprites;
    world.sprites[player].posInTexture = textureRect;
    world.sprites[player].posInWorld = worldRect;
    world.AttachComponent(player, ComponentFlag::Input);
    world.AttachComponent(player, ComponentFlag::Movement);
    world.AttachComponent(player, ComponentFlag::Position);
    world.AttachComponent(player, ComponentFlag::Sprite);
    world.registerPlayerID(player);

    bool bQuit = false;
    while (!bQuit) {
        // Will return Input::Type::Exit if the app needs to exit.
        Input input = playerInputSystem.processInputEvents();
        if (input.type == Input::Exit) {
            break;
        }

        movementSystem.processMovements();

        renderer.Clear();

        renderer.Copy(*(world.sprites[player].texturePtr),
                      world.sprites[player].posInTexture,
                      world.sprites[player].posInWorld);

        renderer.Present();

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
