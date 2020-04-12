#include <GameManager.h>
//#include <Message_generated.h>
//#include <messend.hpp>
//#include <iostream>
//#include <memory>
//#include <stdio.h>

// Anonymous namespace restricts vars to this translation unit
namespace
{
const int SCREEN_WIDTH = 1280;

const int SCREEN_HEIGHT = 720;

/* The number of rows of sprites to render. */
const int VIEWPORT_ROWS = 9;

/* The number of columns of sprites to render. */
const int VIEWPORT_COLUMNS = 16;
}


int main(int argc, char **argv)
{
    NW::GameManager gameManager("New Worlds Client"
                , {SCREEN_WIDTH, SCREEN_HEIGHT}
                , {VIEWPORT_ROWS, VIEWPORT_COLUMNS});
    gameManager.Start();

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
