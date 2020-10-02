#pragma once

#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/array.h"
#include "NetworkDefs.h"
#include "Debug.h"

namespace AM
{

/**
 * This class provides message-related static helper functions for things
 * like serialization and error checking.
 */
class MessageTools
{
public:
    using OutputAdapter = bitsery::OutputBufferAdapter<BinaryBuffer>;
    using InputAdapter = bitsery::InputBufferAdapter<BinaryBuffer>;

    /**
     * Serializes the given object, leaving the results in the given outputBuffer.
     * Relies on the serialization implementation to complain if an invalid type is passed in.
     *
     * @param outputBuffer  The buffer to store the serialized object data in.
     * @param objectToSerialize  The object to serialize. Must be serializable.
     * @param startIndex  Optional, how far into the buffer to start writing the
     *                    serialized bytes.
     * @return The number of bytes written into outputBuffer.
     */
    template <typename T>
    static std::size_t serialize(BinaryBuffer& outputBuffer, T& objectToSerialize,
                                 std::size_t startIndex = 0)
    {
        // Create the adapter manually so we can change the write offset.
        OutputAdapter adapter{outputBuffer};
        adapter.currentWritePos(startIndex);

        // Return value will include the offset, so subtract it back out.
        return (bitsery::quickSerialization<OutputAdapter>(std::move(adapter),
                    objectToSerialize)
                - startIndex);
    }

    template <typename T>
    static bool deserialize(BinaryBuffer& inputBuffer, std::size_t serializedSize,
                            T& outputObject)
    {
        std::pair<bitsery::ReaderError, bool> result = bitsery::quickDeserialization<
            InputAdapter>({inputBuffer.begin(), serializedSize}, outputObject);

        if (!result.second) {
            std::string errorString = "Deserialization failed: ";
            switch (result.first) {
                case bitsery::ReaderError::DataOverflow:
                    errorString += "data overflow.";
                    break;
                case bitsery::ReaderError::InvalidData:
                    errorString += "invalid data.";
                    break;
                case bitsery::ReaderError::InvalidPointer:
                    errorString += "invalid pointer.";
                    break;
                case bitsery::ReaderError::ReadingError:
                    errorString += "reading error.";
                    break;
                default:
                    break;
            }
            DebugError("%s", errorString);
        }
        else {
            return true;
        }
    }

    /**
     * Fills the message header (message type and size) into the given buffer.
     * Also shrinks the buffer to fit the content, if it's over-sized.
     *
     * The first byte at startIndex will contain the message type as a Uint8.
     * The next 2 bytes will contain the message size as a Uint16.
     * The rest will have the data from the given messageBuffer copied into it.
     *
     * @param startIndex  Used to leave room at the front of the message to later be
     *                    filled. The client uses this since it writes the client message
     *                    header into the same buffer. The server doesn't.
     */
    static void fillMessageHeader(MessageType type, std::size_t messageSize,
                                  const BinaryBufferSharedPtr& messageBuffer,
                                  unsigned int startIndex);
};

} // End namespace AM
