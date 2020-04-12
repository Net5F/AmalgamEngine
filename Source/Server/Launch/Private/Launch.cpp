#include <Message_generated.h>
#include <messend.hpp>
#include <iostream>
#include <memory>

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

struct TxData
{
    msnd::Peer* peer;
    msnd::Message message;

    TxData(msnd::Peer* InPeer, msnd::Message InMessage)
    {
        peer = InPeer;
        message = InMessage;
    }
};

static int TransmitThread(void* InData)
{
    flatbuffers::FlatBufferBuilder builder;

    msnd::startup();

    msnd::Acceptor acceptor("127.0.0.1", 41499);

    std::cout << "Waiting for connection.\n";
    std::unique_ptr<msnd::Peer> client = nullptr;
    client = acceptor.acceptWait();
    std::cout << "Connected.\n";

    for (int i = 0; i < 5; i++)
    {
        if (!(client->isConnected()))
        {
            std::cout << "Disconnected.\n";
            break;
        }

        std::cout << "Client connected. Sending data.\n";

        auto position = NW::Vec2(i * 16, i * 16);
        auto messageBuffer = NW::CreateMessage(
                builder, NW::MessageType_Update, &position);
        builder.Finish(messageBuffer);

        msnd::Message message(builder.GetBufferPointer(), builder.GetSize());

        std::unique_ptr<struct TxData> txDataPtr
                = std::make_unique<struct TxData>(client.get(), message);

        client->sendMessage(message);
    }
    std::cout << "Thread done.\n";

    return 0;
}

int main(int argc, char **argv)
{
    SDL_Thread* thread;
    thread = SDL_CreateThread(TransmitThread, "TxMessage", (void*)NULL);

    if (NULL == thread)
    {
        std::cout << "Failed to create thread.\n";
    }

    int threadReturnValue;
    SDL_WaitThread(thread, &threadReturnValue);
    SDLNet_Quit();

    msnd::shutdown();

    std::cout << "Main done.\n";

    return 0;
}
