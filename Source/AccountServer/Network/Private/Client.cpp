#include "Client.h"
#include "Config.h"
#include "ByteTools.h"
#include "Log.h"
#include "asio/read.hpp"
#include "asio/write.hpp"

namespace AM
{
namespace AccountServer
{
Client::Client(NetworkID inNetID, asio::ip::tcp::socket inSocket,
               MessageCallback inMessageCallback,
               DisconnectCallback inDisconnectCallback)
: netID{inNetID}
, socket{std::move(inSocket)}
, messageCallback{std::move(inMessageCallback)}
, disconnectCallback{std::move(inDisconnectCallback)}
, isConnected{true}
, headerBuffer{}
, payloadBuffer{}
, sendQueue{}
, messageTimer{socket.get_executor()}
{
}

void Client::startReceiveLoop()
{
    // Start the first async operation. When it completes, it'll start the next
    // appropriate operation.
    readHeader();
}

void Client::send(BinaryBufferSharedPtr message)
{
    if (!isConnected) {
        return;
    }

    bool writeInProgress{!sendQueue.empty()};
    sendQueue.push(std::move(message));

    if (!writeInProgress) {
        sendNextMessage();
    }
}

void Client::sendNextMessage()
{
    // Note: We need to keep the shared_ptr that owns this class alive, since
    //       the below async operation may be ran after this client is
    //       disconnected (and would otherwise be destroyed).
    auto self{shared_from_this()};

    asio::async_write(socket, asio::buffer(*sendQueue.front()),
                      [self](const asio::error_code& error, std::size_t) {
                          if (!self->isConnected) {
                              return;
                          }
                          if (error) {
                              self->disconnect(error);
                              return;
                          }

                          self->sendQueue.pop();

                          if (!self->sendQueue.empty()) {
                              self->sendNextMessage();
                          }
                      });
}

void Client::readHeader()
{
    if (!isConnected) {
        return;
    }

    auto self{shared_from_this()};

    // Start the message timer, to detect timeouts.
    startMessageTimer();

    asio::async_read(
        socket, asio::buffer(headerBuffer),
        [self](const asio::error_code& error, std::size_t) {
            if (!(self->isConnected)) {
                return;
            }
            if (error) {
                self->disconnect(error);
                return;
            }

            // Validate the header contents.
            Uint8 messageType{
                self->headerBuffer[MessageHeaderIndex::MessageType]};
            Uint16 payloadSize{ByteTools::read16(
                &self->headerBuffer[MessageHeaderIndex::Size])};

            if (payloadSize > CLIENT_MAX_MESSAGE_SIZE) {
                self->disconnect(
                    asio::error::make_error_code(asio::error::message_size));
                return;
            }

            // If the message is 0 bytes, process it and queue the next header
            // read.
            if (payloadSize == 0) {
                self->messageTimer.cancel();
                self->messageCallback(self->netID, messageType, {});

                // Message processing could have caused a disconnect.
                if (self->isConnected) {
                    self->readHeader();
                }
                return;
            }

            self->readPayload(messageType, payloadSize);
        });
}

void Client::readPayload(Uint8 messageType, Uint16 payloadSize)
{
    if (!isConnected) {
        return;
    }

    auto self{shared_from_this()};
    payloadBuffer.resize(payloadSize);
    asio::async_read(
        socket, asio::buffer(payloadBuffer),
        [self, messageType](const asio::error_code& error, std::size_t) {
            if (!(self->isConnected)) {
                return;
            }
            if (error) {
                self->disconnect(error);
                return;
            }

            // Message received. Pass it up to ClientManager for processing.
            self->messageTimer.cancel();
            self->messageCallback(self->netID, messageType,
                                  self->payloadBuffer);

            // Message processing could have caused a disconnect.
            if (self->isConnected) {
                self->readHeader();
            }
        });
}

void Client::startMessageTimer()
{
    messageTimer.expires_after(
        std::chrono::duration_cast<asio::steady_timer::duration>(
            std::chrono::duration<double>(Config::CLIENT_TIMEOUT_S)));

    auto self{shared_from_this()};
    messageTimer.async_wait([self](const asio::error_code& error) {
        // operation_aborted means the message completed and cancelled/reset
        // this timer.
        if (!(self->isConnected) || (error == asio::error::operation_aborted)) {
            return;
        }
        if (error) {
            self->disconnect(error);
            return;
        }

        self->disconnect(
            asio::error::make_error_code(asio::error::timed_out));
    });
}

void Client::disconnect(const asio::error_code& error)
{
    if (!isConnected) {
        return;
    }

    isConnected = false;

    messageTimer.cancel();

    // close() cancels outstanding reads and writes. Their handlers will
    // eventually receive operation_aborted.
    asio::error_code ignoredError{};
    socket.close(ignoredError);

    // Tell ClientManager to handle the disconnect.
    disconnectCallback(netID, error);
}

} // end namespace AccountServer
} // end namespace AM
