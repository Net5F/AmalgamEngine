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
     */
    template <typename T>
    static std::size_t serialize(BinaryBuffer& outputBuffer, T& objectToSerialize)
    {
        return bitsery::quickSerialization<OutputAdapter>(outputBuffer, objectToSerialize);
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

private:
};

} // End namespace AM
