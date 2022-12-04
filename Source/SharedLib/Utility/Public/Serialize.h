#pragma once

#include "SerializeBuffer.h"
#include "Log.h"
#include <SDL_stdinc.h>
#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/adapter/stream.h"
#include "bitsery/adapter/measure_size.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/array.h"
#include "bitsery/traits/string.h"
#include <fstream>

namespace AM
{
class Serialize
{
public:
    using OutputAdapter = bitsery::OutputBufferAdapter<SerializeBuffer>;

    /**
     * Serializes the given object, writing the serialized bytes into the given
     * outputBuffer.
     *
     * Relies on the serialization implementation to complain if an invalid
     * type is passed in.
     *
     * @param outputBuffer  The buffer to store the serialized object data in.
     * @param bufferSize  The size of the output buffer.
     * @param objectToSerialize  The object to serialize. Must be serializable.
     * @param startIndex  Optional, how far into the buffer to start writing the
     *                    serialized bytes.
     * @return The number of bytes written into outputBuffer.
     */
    template<typename T>
    static std::size_t toBuffer(Uint8* outputBuffer, std::size_t bufferSize,
                                T& objectToSerialize,
                                std::size_t startIndex = 0)
    {
        // Note: In Debug, Bitsery will assert if the serialized object size
        //       is larger than bufferSize.

        // Create the adapter manually so we can change the write offset.
        SerializeBuffer buffer{outputBuffer, bufferSize};
        OutputAdapter adapter{buffer};
        adapter.currentWritePos(startIndex);

        // Serialize and return.
        // Note: The return value will include the offset, so subtract it back
        //       out.
        return (bitsery::quickSerialization<OutputAdapter>(std::move(adapter),
                                                           objectToSerialize)
                - startIndex);
    }

    /**
     * Serializes the given object, writing the serialized bytes into the
     * given file.
     *
     * @param filePath  The file to write to.
     * @param objectToSerialize  The object to serialize. Must be serializable.
     * @return false if the file failed to open, else true.
     */
    template<typename T>
    static bool toFile(const std::string& filePath, T& objectToSerialize)
    {
        // Open or create the file.
        std::ofstream file(filePath, std::ios::binary);
        if (!(file.is_open())) {
            LOG_ERROR("Failed to open file: %s", filePath.c_str());
            return false;
        }

        // Initialize the stream serializer.
        bitsery::Serializer<bitsery::OutputBufferedStreamAdapter> serializer{
            file};
        serializer.object(objectToSerialize);

        // Serialize the object.
        serializer.adapter().flush();

        return true;
    }

    /**
     * Measures what the serialized size of the given object will be.
     * @return The serialized size of the object, in bytes.
     */
    template<typename T>
    static std::size_t measureSize(T& objectToSerialize)
    {
        return bitsery::quickSerialization(bitsery::MeasureSize{},
                                           objectToSerialize);
    }
};

} // End namespace AM
