#pragma once

#include "entt/resource/handle.hpp"
#include "SDL2pp/Texture.hh"

namespace AM
{

// Convenience alias for texture resource handle (since
// entt::resource_handle<TypeName> is very verbose).
using TextureHandle = entt::resource_handle<SDL2pp::Texture>;

} // End namespace AM


namespace entt {

/**
 * Full specialization of entt::resource_handle for texture resources.
 *
 * Copy+paste of original class, but adds getSharedPtr() to facilitate sharing
 * lifetime ownership with other libraries without forcing an entt dependency
 * on them.
 */
template<>
class resource_handle<SDL2pp::Texture> {
    using Resource = SDL2pp::Texture;

    /*! @brief Resource handles are friends of their caches. */
    friend struct resource_cache<Resource>;

    resource_handle(std::shared_ptr<Resource> res) ENTT_NOEXCEPT
        : resource{std::move(res)}
    {}

public:
    /*! @brief Default constructor. */
    resource_handle() ENTT_NOEXCEPT = default;

    /**
     * @brief Gets a reference to the managed resource.
     *
     * @warning
     * The behavior is undefined if the handle doesn't contain a resource.
     *
     * @return A reference to the managed resource.
     */
    [[nodiscard]] const Resource & get() const ENTT_NOEXCEPT {
        ENTT_ASSERT(static_cast<bool>(resource));
        return *resource;
    }

    /*! @copydoc get */
    [[nodiscard]] Resource & get() ENTT_NOEXCEPT {
        return const_cast<Resource &>(std::as_const(*this).get());
    }

    /*! @copydoc get */
    [[nodiscard]] operator const Resource & () const ENTT_NOEXCEPT {
        return get();
    }

    /*! @copydoc get */
    [[nodiscard]] operator Resource & () ENTT_NOEXCEPT {
        return get();
    }

    /*! @copydoc get */
    [[nodiscard]] const Resource & operator *() const ENTT_NOEXCEPT {
        return get();
    }

    /*! @copydoc get */
    [[nodiscard]] Resource & operator *() ENTT_NOEXCEPT {
        return get();
    }

    /**
     * @brief Gets a pointer to the managed resource.
     *
     * @warning
     * The behavior is undefined if the handle doesn't contain a resource.
     *
     * @return A pointer to the managed resource or `nullptr` if the handle
     * contains no resource at all.
     */
    [[nodiscard]] const Resource * operator->() const ENTT_NOEXCEPT {
        ENTT_ASSERT(static_cast<bool>(resource));
        return resource.get();
    }

    /*! @copydoc operator-> */
    [[nodiscard]] Resource * operator->() ENTT_NOEXCEPT {
        return const_cast<Resource *>(std::as_const(*this).operator->());
    }

    /**
     * @brief Returns true if a handle contains a resource, false otherwise.
     * @return True if the handle contains a resource, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const ENTT_NOEXCEPT {
        return static_cast<bool>(resource);
    }

    /**
     * @brief Gets a reference to the underlying shared_ptr.
     *
     * @warning
     * The behavior is undefined if the handle doesn't contain a resource.
     *
     * @return A reference to the underlying shared_ptr.
     */
    [[nodiscard]] const std::shared_ptr<Resource>& getSharedPtr() const ENTT_NOEXCEPT {
        return resource;
    }

private:
    std::shared_ptr<Resource> resource;
};

} // namespace entt
