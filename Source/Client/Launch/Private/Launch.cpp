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
const int SCREEN_WIDTH = 1280;

const int SCREEN_HEIGHT = 720;
}

class Component
{
public:
    // The indices in each Entity's components array that each component resides at.
    enum Indices
    {
        Position,
        Movement,
        Input,
        Sprite,
        NumComponents
    };
};

class PositionComponent : Component
{
public:
    PositionComponent(float inX, float inY)
    : x(inX)
    , y(inY)
    {
    }

    /** Current position. */
    float x;
    float y;
};

class MovementComponent : Component
{
public:
    MovementComponent(float inVelX, float inVelY)
    : velX(inVelX)
    , velY(inVelY)
    {
    }

    /** Current velocities. */
    float velX;
    float velY;
};

class InputComponent : Component
{
public:
    enum class Input
    {
        None,
        Up,
        Down,
        Left,
        Right,
        Exit
    };

    /** The last received input. */
    Input currentInput;
};

class SpriteComponent : Component
{
public:
    SpriteComponent(SDL2pp::Texture& inTexture, SDL2pp::Rect inPosInTexture, SDL2pp::Rect inPosInWorld)
    : texture(inTexture)
    , posInTexture(inPosInTexture)
    , posInWorld(inPosInWorld)
    {
    }

    /** A reference to the texture that holds this sprite. */
    SDL2pp::Texture& texture;

    /** UV position and size in texture. */
    SDL2pp::Rect posInTexture;

    /** ST position and size in world. */
    SDL2pp::Rect posInWorld;
};

class IDPool
{
public:
    static uint32_t getID()
    {
        for (uint16_t i = 0; i < MAX_IDS; ++i) {
            // Find the first false.
            if (!(IDs[i])) {
                IDs[i] = true;
                return i;
            }
        }

        std::cerr << "Tried to get ID when all were taken. Returning 0." << std::endl;
        return 0;
    }

    static void resetID(uint32_t ID)
    {
        if (IDs[ID]) {
            IDs[ID] = false;
        }
        else {
            std::cerr << "Tried to reset an unused ID." << std::endl;
        }
    }

private:
    static constexpr uint16_t MAX_IDS = 100;

    /**
     * If ID 'x' is available, IDs[x] will be true. Else, it will be false.
     */
    static std::array<bool, MAX_IDS> IDs;
};
std::array<bool, IDPool::MAX_IDS> IDPool::IDs = {}; // Init to 0;

class Entity
{
public:
    Entity()
    : ID(IDPool::getID())
    {
        components.resize(Component::NumComponents);
    };

    void addComponent(std::unique_ptr<Component> component, Component::Indices index)
    {
        if (components[index] == nullptr) {
            components[index] = std::move(component);
        }
        else {
            std::cerr << "Tried to add a component where one already existed." << std::endl;
        }
    }

    void remComponent(Component::Indices index)
    {
        if (components[index] != nullptr) {
            components[index] = nullptr;
        }
        else {
            std::cerr << "Tried to remove a component where one didn't exist." << std::endl;
        }
    }

    uint32_t ID;

    /** The components that the Entity possesses. The ComponentTypes enum accesses the proper index for each component. */
    std::vector<std::unique_ptr<Component>> components;
};

int main(int argc, char **argv) try
{
    // Setup the SDL constructs.
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Amalgam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL2pp::Texture sprites(renderer, "Resources/u4_tiles_pc_ega.png");

    // Calc the center of the screen.
    int centerX = renderer.GetOutputWidth() / 2;
    int centerY = renderer.GetOutputHeight() / 2;

    // Setup our player.
    Entity player;
    SDL2pp::Rect textureRect(0, 32, 16, 16);
    SDL2pp::Rect worldRect(centerX - 64, centerY - 64, 64, 64);
    player.addComponent(std::unique_ptr<Component>(new PositionComponent(40, 40)), Component::Position);
    player.addComponent(std::unique_ptr<Component>(new MovementComponent(0, 0)), Component::Movement);
    player.addComponent(std::unique_ptr<Component>(new InputComponent(InputComponent::Input::None)), Component::Input);
    player.addComponent(std::unique_ptr<Component>(new SpriteComponent(sprites, textureRect, worldRect)), Component::Sprite);

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

        renderer.Copy(sprites, SDL2pp::Rect(0, 32, 16, 16), SDL2pp::Rect(centerX - 64, centerY - 64, 64, 64));

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
