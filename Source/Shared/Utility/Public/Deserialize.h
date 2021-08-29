#pragma once

#include "BinaryBuffer.h"
#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/array.h"
#include "bitsery/traits/string.h"
#include "Log.h"

namespace AM
{

class Deserialize
{
public:
    using InputAdapter = bitsery::InputBufferAdapter<BinaryBuffer>;

    /**
     * Deserializes the contents of the given buffer into the given object.
     * Errors if deserialization fails.
     *
     * @param inputBuffer  A buffer containing the serialized bytes to
     * deserialize.
     * @param serializedSize  The size of the serialized message.
     * @param outputObject  The object to store the deserialized message in.
     * @param startIndex  Optional, how far into the buffer to start reading
     * bytes from.
     */
    template<typename T>
    static bool fromBuffer(const BinaryBuffer& inputBuffer,
                            std::size_t serializedSize, T& outputObject,
                            std::size_t startIndex = 0)
    {
        std::pair<bitsery::ReaderError, bool> result
            = bitsery::quickDeserialization<InputAdapter>(
                {inputBuffer.begin() + startIndex, serializedSize},
                outputObject);

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
            // TODO: Eventually change this to a LOG_INFO and return false.
            LOG_ERROR("%s", errorString);
        }
        else {
            return true;
        }
    }
};

} // End namespace AM
