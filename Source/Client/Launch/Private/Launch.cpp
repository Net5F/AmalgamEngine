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
#include <algorithm>
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
    {
    }

    /** Current velocities. */
    float velX;
    float velY;
};

enum class Input
{
    None,
    Up,
    Down,
    Left,
    Right,
    Exit
};

struct InputComponent
{
public:
    InputComponent()
    : currentInput(Input::None)
    {
    }

    /** The last received input. */
    Input currentInput;
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

class World
{
public:
    enum ComponentType
    {
        PositionType = 1 << 0,
        MovementType = 1 << 1,
        InputType = 1 << 2,
        SpriteType = 1 << 3
    };

    EntityID AddEntity(const std::string& name)
    {
        EntityID id = IDPool::reserveID();
        entityNames[id] = name;

        return id;
    }

    /** Entity data lists. */
    std::array<std::string, MAX_ENTITIES> entityNames;
    std::array<PositionComponent, MAX_ENTITIES> positions;
    std::array<MovementComponent, MAX_ENTITIES> movements;
    std::array<InputComponent, MAX_ENTITIES> inputs;
    std::array<SpriteComponent, MAX_ENTITIES> sprites;
    // Bit flags for every component, indicating whether the object at a given index has that component.
    std::array<uint32_t, MAX_ENTITIES> componentTypes;
};

class InputSystem
{
public:
    void processLatestInputs()
    {
    }
};

class MovementSystem
{
public:
    void processMovements()
    {
    }
};

class RenderSystem
{
public:
    void collectRenderObjects()
    {
    }

    void render()
    {
    }
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

    // Setup our player.
    SDL2pp::Rect textureRect(0, 32, 16, 16);
    SDL2pp::Rect worldRect(centerX - 64, centerY - 64, 64, 64);

    EntityID playerID = world.AddEntity("Player");
    world.positions[playerID].x = centerX - 64;
    world.positions[playerID].y = centerY - 64;
    world.sprites[playerID].texturePtr = sprites;
    world.sprites[playerID].posInTexture = textureRect;
    world.sprites[playerID].posInWorld = worldRect;

    bool bQuit = false;
    while (!bQuit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                bQuit = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        bQuit = true;
                }
            }
        }

        renderer.Clear();

        renderer.Copy(*(world.sprites[playerID].texturePtr)
                      , world.sprites[playerID].posInTexture
                      , world.sprites[playerID].posInWorld);

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
