#include <catch2/catch.hpp>
#include "MessageSorter.h"
#include "NetworkDefs.h"
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
        messageSorter.push(
            0, std::make_unique<BinaryBuffer>(BinaryBuffer{1, 2, 3}));

        // Try to receive.
        std::queue<BinaryBufferPtr>& queue = messageSorter.startReceive(0);
        REQUIRE(!(queue.empty()));

        BinaryBufferPtr message = std::move(queue.front());
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(!(messageSorter.isTickValid(0)));
    }

    SECTION("Multiple messages for same tick.")
    {
        // Push 10 messages into tick 0.
        for (Uint8 i = 0; i < 10; ++i) {
            messageSorter.push(0, std::make_unique<std::vector<Uint8>>(
                                      std::vector<Uint8>{i, i, i}));
        }

        // Try to receive.
        std::queue<BinaryBufferPtr>& queue = messageSorter.startReceive(0);
        REQUIRE(queue.size() == 10);

        Uint8 counter = 0;
        while (!queue.empty()) {
            // Check that the message contents are correct.
            BinaryBufferPtr message = std::move(queue.front());
            REQUIRE(*message
                    == std::vector<Uint8>({counter, counter, counter}));

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
                messageSorter.push(tick, std::make_unique<std::vector<Uint8>>(
                                             std::vector<Uint8>{i, i, i}));
            }
        }

        // Try to receive tick 0.
        std::queue<BinaryBufferPtr>* queue = &(messageSorter.startReceive(0));
        REQUIRE(queue->size() == 10);

        Uint8 counter = 0;
        while (!(queue->empty())) {
            // Check that the message contents are correct.
            BinaryBufferPtr message = std::move(queue->front());
            REQUIRE(*message
                    == std::vector<Uint8>({counter, counter, counter}));

            // Prep for the next message.
            queue->pop();
            counter++;
        }

        messageSorter.endReceive();
        REQUIRE(messageSorter.getCurrentTick() == 1);
        REQUIRE(!(messageSorter.isTickValid(0)));

        // Check that tick 1 is empty.
        queue = &(messageSorter.startReceive(1));
        REQUIRE(queue->size() == 0);

        messageSorter.endReceive();
        REQUIRE(messageSorter.getCurrentTick() == 2);
        REQUIRE(!(messageSorter.isTickValid(1)));

        // Try to receive ticks 2 and 3.
        for (Uint32 i = 2; i <= 3; ++i) {
            queue = &(messageSorter.startReceive(i));
            REQUIRE(queue->size() == 10);

            Uint8 counter = 0;
            while (!(queue->empty())) {
                // Check that the message contents are correct.
                BinaryBufferPtr message = std::move(queue->front());
                REQUIRE(*message
                        == std::vector<Uint8>({counter, counter, counter}));

                // Prep for the next message.
                queue->pop();
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
        REQUIRE(messageSorter.getCurrentTick()
                == (Server::MessageSorter::BUFFER_SIZE));
        REQUIRE(!(
            messageSorter.isTickValid(Server::MessageSorter::BUFFER_SIZE - 1)));
        REQUIRE((messageSorter.isTickValid(
            Server::MessageSorter::BUFFER_SIZE * 2 - 1)));

        // Push a message.
        messageSorter.push(
            Server::MessageSorter::BUFFER_SIZE,
            std::make_unique<std::vector<Uint8>>(std::vector<Uint8>{1, 2, 3}));

        // Try to receive.
        std::queue<BinaryBufferPtr>& queue
            = messageSorter.startReceive(Server::MessageSorter::BUFFER_SIZE);
        REQUIRE(!(queue.empty()));

        BinaryBufferPtr message = std::move(queue.front());
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(
            !(messageSorter.isTickValid(Server::MessageSorter::BUFFER_SIZE)));
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
        messageSorter.push(tickNum, std::make_unique<std::vector<Uint8>>(
                                        std::vector<Uint8>{1, 2, 3}));

        // Try to receive.
        std::queue<BinaryBufferPtr>& queue
            = messageSorter.startReceive(tickNum);
        REQUIRE(!(queue.empty()));

        BinaryBufferPtr message = std::move(queue.front());
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
        messageSorter.push(7, std::make_unique<std::vector<Uint8>>(
                                  std::vector<Uint8>{1, 2, 3}));

        // Advance to tick 7.
        for (unsigned int i = 2; i < 7; ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }

        // Try to receive.
        std::queue<BinaryBufferPtr>& queue = messageSorter.startReceive(7);
        REQUIRE(!(queue.empty()));

        BinaryBufferPtr message = std::move(queue.front());
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        // Advance the tick.
        messageSorter.endReceive();
        REQUIRE(!(messageSorter.isTickValid(7)));
    }

    SECTION("Large positive difference returns invalid.")
    {
        // Create a large diff.
        Sint64 result = messageSorter.push(
            5000, std::make_unique<BinaryBuffer>(BinaryBuffer{1, 2, 3}));

        REQUIRE(result == 5000);
        REQUIRE(!(messageSorter.isTickValid(5000)));
    }

    SECTION("Large negative difference returns invalid.")
    {
        // Create a large negative diff.
        for (unsigned int i = 0; i < 1000; ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }

        Sint64 result = messageSorter.push(
            100, std::make_unique<BinaryBuffer>(BinaryBuffer{1, 2, 3}));

        REQUIRE(result == -900);
        REQUIRE(!(messageSorter.isTickValid(100)));
    }

    SECTION("Small positive difference returns invalid.")
    {
        // Create a small diff.
        Sint64 result1 = messageSorter.push(
            0, std::make_unique<BinaryBuffer>(BinaryBuffer{1, 2, 3}));

        Sint64 result2 = messageSorter.push(
            Server::MessageSorter::BUFFER_SIZE - 1,
            std::make_unique<BinaryBuffer>(BinaryBuffer{1, 2, 3}));

        REQUIRE(result1 == 0);
        REQUIRE(result2 == (Server::MessageSorter::BUFFER_SIZE - 1));
        REQUIRE(
            messageSorter.isTickValid(Server::MessageSorter::BUFFER_SIZE - 1));
        REQUIRE(
            !(messageSorter.isTickValid(Server::MessageSorter::BUFFER_SIZE)));
    }

    SECTION("Small negative difference returns invalid.")
    {
        // Create a small negative diff.
        for (unsigned int i = 0; i < 1000; ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }

        Sint64 result1 = messageSorter.push(
            1000, std::make_unique<BinaryBuffer>(BinaryBuffer{1, 2, 3}));

        Sint64 result2 = messageSorter.push(
            1000 - 1, std::make_unique<BinaryBuffer>(BinaryBuffer{1, 2, 3}));

        REQUIRE(result1 == 0);
        REQUIRE(result2 == -1);
        REQUIRE(messageSorter.isTickValid(1000));
        REQUIRE(!(messageSorter.isTickValid(1000 - 1)));
    }

    SECTION("Wrapping occurs at the expected point.")
    {
        // Push a message (we're going to leave it as a marker for wrapping.)
        messageSorter.push(0, std::make_unique<std::vector<Uint8>>(
                                  std::vector<Uint8>{1, 2, 3}));

        // Advance until we're on the edge of wrapping.
        for (unsigned int i = 0; i < (Server::MessageSorter::BUFFER_SIZE - 1);
             ++i) {
            messageSorter.startReceive(i);
            messageSorter.endReceive();
        }
        REQUIRE(messageSorter.getCurrentTick()
                == (Server::MessageSorter::BUFFER_SIZE - 1));

        // Try to receive, there should be nothing here.
        std::queue<BinaryBufferPtr>* queue = &(
            messageSorter.startReceive(Server::MessageSorter::BUFFER_SIZE - 1));
        REQUIRE(queue->empty());

        // Advance over the edge of wrapping.
        messageSorter.endReceive();

        // Try to receive, since we should have wrapped back to the original
        // message.
        queue
            = &(messageSorter.startReceive(Server::MessageSorter::BUFFER_SIZE));
        REQUIRE(!(queue->empty()));

        BinaryBufferPtr message = std::move(queue->front());
        REQUIRE(*message == std::vector<Uint8>({1, 2, 3}));

        messageSorter.endReceive();
    }
}
