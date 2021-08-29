#pragma once

#include "BinaryBuffer.h"
#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/adapter/measure_size.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/array.h"
#include "bitsery/traits/string.h"
#include "Log.h"

namespace AM
{

class Serialize
{
public:
    using OutputAdapter = bitsery::OutputBufferAdapter<BinaryBuffer>;

    /**
     * Serializes the given object, leaving the results in the given
     * outputBuffer. Relies on the serialization implementation to complain if
     * an invalid type is passed in.
     *
     * @param outputBuffer  The buffer to store the serialized object data in.
     * @param objectToSerialize  The object to serialize. Must be serializable.
     * @param startIndex  Optional, how far into the buffer to start writing the
     *                    serialized bytes.
     * @return The number of bytes written into outputBuffer.
     */
    template<typename T>
    static std::size_t toBuffer(BinaryBuffer& outputBuffer,
                                 T& objectToSerialize,
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

    /**
     * Measures what the serialized size of the given object will be.
     * @return The serialized size of the object, in bytes.
     */
    template<typename T>
    static std::size_t measureSize(T& objectToSerialize)
    {
        return bitsery::quickSerialization(bitsery::MeasureSize{}, objectToSerialize);
    }
};

} // End namespace AM
