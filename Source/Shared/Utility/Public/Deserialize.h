#pragma once

#include "BinaryBuffer.h"
#include "Log.h"
#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/adapter/stream.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/array.h"
#include "bitsery/traits/string.h"
#include <fstream>

namespace AM
{
class Deserialize
{
public:
    using InputAdapter = bitsery::InputBufferAdapter<const Uint8*>;

    /**
     * Deserializes the contents of the given buffer into the given object.
     *
     * Errors if deserialization fails.
     *
     * @param inputBuffer  A buffer containing the serialized bytes to
     *                     deserialize.
     * @param serializedSize  The size, in bytes, of the serialized object.
     * @param outputObject  The object to store the deserialized data in.
     * @param startIndex  Optional, how far into the buffer to start reading
     *                    bytes from.
     */
    template<typename T>
    static bool fromBuffer(const Uint8* inputBuffer,
                           std::size_t serializedSize, T& outputObject,
                           std::size_t startIndex = 0)
    {
        // Deserialize the buffer contents into outputObject.
        std::pair<bitsery::ReaderError, bool> result
            = bitsery::quickDeserialization<InputAdapter>(
                {inputBuffer + startIndex, serializedSize},
                outputObject);

        // If there was an error, print it and fail.
        if (!result.second) {
            std::string errorString{getErrorString(result.first)};

            // TODO: Eventually change this to a LOG_INFO and return false.
            LOG_ERROR("%s", errorString.c_str());
        }
        else {
            return true;
        }
    }

    /**
     * Deserializes the contents of the given file into the given object.
     *
     * Errors if the file cannot be opened or deserialization fails.
     *
     * @param filePath  The file to read from.
     * @param outputObject  The object to store the deserialized data in.
     */
    template<typename T>
    static bool fromFile(const std::string& filePath, T& outputObject)
    {
        // Open the file.
        std::ifstream file(filePath, std::ios::binary);
        if (!(file.is_open())) {
            LOG_ERROR("Could not open file for deserialization: %s",
                      filePath.c_str());
        }

        // Deserialize the file contents into outputObject.
        std::pair<bitsery::ReaderError, bool> result
            = bitsery::quickDeserialization<bitsery::InputStreamAdapter>(
                file, outputObject);

        // If there was an error, print it and fail.
        if (!result.second) {
            std::string errorString{getErrorString(result.first)};

            // TODO: Eventually change this to a LOG_INFO and return false.
            LOG_ERROR("%s", errorString.c_str());
        }
        else {
            return true;
        }
    }

private:
    static std::string getErrorString(bitsery::ReaderError error)
    {
        std::string errorString{"Deserialization failed: "};
        switch (error) {
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

        return errorString;
    }
};

} // End namespace AM
