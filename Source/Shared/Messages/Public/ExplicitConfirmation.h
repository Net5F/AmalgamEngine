#pragma once

/**
 * This message would normally have a struct and all, but because it's created
 * in the network layer and our normal message send/receive path involves
 * BinaryBufferSharedPtr (which would be a waste to allocate when we're
 * already dealing with the batch buffer), we instead just write/read it by
 * hand.
 *
 * Over the wire, it still has the normal message header and uses
 * MessageType::ExplicitConfirmation.
 *
 * See Server::Client::addExplicitConfirmation() for the send code, and
 * Client::MessageHandler::handleExplicitConfirmation for the receive code.
 */
