#include <catch2/catch.hpp>
#include "MessageSorter.h"
#include "SharedDefs.h"
#include <memory>
#include <queue>
#include "Debug.h"

using namespace AM;

TEST_CASE("TestMessageSorter")
{
    Server::MessageSorter messageSorter;

    REQUIRE(messageSorter.getCurrentTick() == 0);

    SECTION("Single message.")
    {
        // Push a message into tick 0.
        messageSorter.push(0,
            std::make_shared<std::vector<Uint8>>(std::vector<Uint8>{1, 2, 3}));

        // Try to receive.
        std::queue<BinaryBufferSharedPtr>& queue = messageSorter.startReceive(0);
        REQUIRE(!(queue.empty()));

        BinaryBufferSharedPtr message = queue.front();
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(!(messageSorter.isTickValid(0)));
    }

    SECTION("Multiple messages for same tick.")
    {
        // Push 10 messages into tick 0.
        for (Uint8 i = 0; i < 10; ++i) {
            messageSorter.push(0,
                std::make_shared<std::vector<Uint8>>(std::vector<Uint8>{i, i, i}));
        }

        // Try to receive.
        std::queue<BinaryBufferSharedPtr>& queue = messageSorter.startReceive(0);
        REQUIRE(queue.size() == 10);

        Uint8 counter = 0;
        while (!queue.empty()) {
            // Check that the message contents are correct.
            BinaryBufferSharedPtr message = queue.front();
            REQUIRE(*message == std::vector<Uint8>({counter, counter, counter}));

            // Prep for the next message.
            queue.pop();
            counter++;
        }

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(!(messageSorter.isTickValid(0)));
    }

    SECTION("Multiple messages over multiple ticks.")
    {
        // Push 10 messages into ticks 0, 2, and 3.
        std::array<int, 3> ticksToUse = {0, 2, 3};
        int temp = 0;
        for (int tick : ticksToUse) {
            for (Uint8 i = 0; i < 10; ++i) {
                temp++;
                messageSorter.push(tick,
                    std::make_shared<std::vector<Uint8>>(std::vector<Uint8>{i, i, i}));
            }
        }

        // Try to receive tick 0.
        std::queue<BinaryBufferSharedPtr>& queue = messageSorter.startReceive(0);
        REQUIRE(queue.size() == 10);

        Uint8 counter = 0;
        while (!(queue.empty())) {
            // Check that the message contents are correct.
            BinaryBufferSharedPtr message = queue.front();
            REQUIRE(*message == std::vector<Uint8>({counter, counter, counter}));

            // Prep for the next message.
            queue.pop();
            counter++;
        }

        messageSorter.endReceive();
        REQUIRE(messageSorter.getCurrentTick() == 1);
        REQUIRE(!(messageSorter.isTickValid(0)));

        // Check that tick 1 is empty.
        queue = messageSorter.startReceive(1);
        REQUIRE(queue.size() == 0);

        messageSorter.endReceive();
        REQUIRE(messageSorter.getCurrentTick() == 2);
        REQUIRE(!(messageSorter.isTickValid(1)));

        // Try to receive ticks 2 and 3.
        for (Uint32 i = 2; i <= 3; ++i) {
            queue = messageSorter.startReceive(i);
            REQUIRE(queue.size() == 10);

            Uint8 counter = 0;
            while (!(queue.empty())) {
                // Check that the message contents are correct.
                BinaryBufferSharedPtr message = queue.front();
                REQUIRE(*message == std::vector<Uint8>({counter, counter, counter}));

                // Prep for the next message.
                queue.pop();
                counter++;
            }

            messageSorter.endReceive();
            REQUIRE(messageSorter.getCurrentTick() == (i + 1));
            REQUIRE(!(messageSorter.isTickValid(i)));
        }
    }

    SECTION("Wrap once.")
    {
        // Advance until we wrap.
        for (unsigned int i = 0; i < Server::MessageSorter::BUFFER_SIZE; ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }

        // Check that we're on the correct tick and everything works.
        REQUIRE(messageSorter.getCurrentTick() == (Server::MessageSorter::BUFFER_SIZE));
        REQUIRE(!(messageSorter.isTickValid(Server::MessageSorter::BUFFER_SIZE - 1)));
        REQUIRE((messageSorter.isTickValid(Server::MessageSorter::BUFFER_SIZE * 2 - 1)));

        // Push a message.
        messageSorter.push(Server::MessageSorter::BUFFER_SIZE,
            std::make_shared<std::vector<Uint8>>(std::vector<Uint8>{1, 2, 3}));

        // Try to receive.
        std::queue<BinaryBufferSharedPtr>& queue
            = messageSorter.startReceive(Server::MessageSorter::BUFFER_SIZE);
        REQUIRE(!(queue.empty()));

        BinaryBufferSharedPtr message = queue.front();
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(!(messageSorter.isTickValid(Server::MessageSorter::BUFFER_SIZE)));
    }

    SECTION("Wrap a lot.")
    {
        // Advance until we wrap.
        unsigned int tickNum = Server::MessageSorter::BUFFER_SIZE * 407;
        for (unsigned int i = 0; i < tickNum; ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }

        // Check that we're on the correct tick and everything works.
        REQUIRE(messageSorter.getCurrentTick() == (tickNum));
        REQUIRE(!(messageSorter.isTickValid(tickNum - 1)));
        REQUIRE((messageSorter.isTickValid(
            tickNum + Server::MessageSorter::BUFFER_SIZE - 1)));

        // Push a message.
        messageSorter.push(tickNum,
            std::make_shared<std::vector<Uint8>>(std::vector<Uint8>{1, 2, 3}));

        // Try to receive.
        std::queue<BinaryBufferSharedPtr>& queue
            = messageSorter.startReceive(tickNum);
        REQUIRE(!(queue.empty()));

        BinaryBufferSharedPtr message = queue.front();
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(!(messageSorter.isTickValid(tickNum)));
    }

    SECTION("Pushing from non-zero indices works correctly.")
    {
        // Advance to tick 2.
        for (unsigned int i = 0; i < 2; ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }

        // Push a message into tick 7.
        messageSorter.push(7,
            std::make_shared<std::vector<Uint8>>(std::vector<Uint8>{1, 2, 3}));

        // Advance to tick 7.
        for (unsigned int i = 2; i < 7; ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }

        // Try to receive.
        std::queue<BinaryBufferSharedPtr>& queue = messageSorter.startReceive(7);
        REQUIRE(!(queue.empty()));

        BinaryBufferSharedPtr message = queue.front();
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(!(messageSorter.isTickValid(7)));
    }
}
