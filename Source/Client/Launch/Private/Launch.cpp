//#include <Message_generated.h>
//#include <messend.hpp>
//#include <iostream>
//#include <memory>
//#include <stdio.h>
#include <stdio.h>
#include <string>
#include <exception>
#include <SDL2pp/SDL2pp.hh>

// Anonymous namespace restricts vars to this translation unit
namespace
{
const int SCREEN_WIDTH = 1280;

const int SCREEN_HEIGHT = 720;
}

struct PositionComponent
{
    float x;
    float y;
};

struct MovementComponent
{
    float velx;
    float vely;
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
    // Stores the last received input.
    Input currentInput;
};

int main(int argc, char **argv) try
{
    // Setup the renderer.
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Amalgam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL2pp::Texture sprites(renderer, "Resources/u4_tiles_pc_ega.png");

    int centerX = renderer.GetOutputWidth() / 2;
    int centerY = renderer.GetOutputHeight() / 2;

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
