target_sources(SharedLib
    PRIVATE
        Private/AssetCache.cpp
        Private/ByteTools.cpp
        Private/IDPool.cpp
        Private/Log.cpp
        Private/Paths.cpp
        Private/PeriodicCaller.cpp
        Private/SDLHelpers.cpp
        Private/Timer.cpp
        Private/Transforms.cpp
    PUBLIC
        Public/AMAssert.h
        Public/AssetCache.h
        Public/BinaryBuffer.h
        Public/ByteTools.h
        Public/ConstexprTools.h
        Public/Deserialize.h
        Public/HashTools.h
        Public/IDPool.h
        Public/OSEventHandler.h
        Public/Log.h
        Public/Paths.h
        Public/PeriodicCaller.h
        Public/SDLHelpers.h
        Public/Serialize.h
        Public/SerializeBuffer.h
        Public/Timer.h
        Public/Transforms.h
)

target_include_directories(SharedLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

# Build our precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(SharedLib PRIVATE Private/pch.h)
endif()
