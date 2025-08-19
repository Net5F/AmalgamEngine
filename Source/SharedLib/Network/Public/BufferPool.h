#pragma once

#include <array>
#include <memory>
#include <vector>

namespace AM
{
/**
 * A pool of reusable data buffers.
 */
template<std::size_t BUFFER_SIZE>
class BufferPool
{
public:
    using BufferType = std::array<Uint8, BUFFER_SIZE>;

    std::unique_ptr<BufferType> acquire()
    {
        if (!(pool.empty())) {
            auto buffer{std::move(pool.back())};
            pool.pop_back();
            return buffer;
        }

        return std::make_unique<BufferType>();
    }

    void release(std::unique_ptr<BufferType> buffer)
    {
        pool.emplace_back(std::move(buffer));
    }

private:
    std::vector<std::unique_ptr<BufferType>> pool{};
};

} /* End namespace AM */
