target_sources(Shared
    PRIVATE
        Private/AssetCache.cpp
        Private/ByteTools.cpp
        Private/IDPool.cpp
        Private/Log.cpp
        Private/Paths.cpp
        Private/PeriodicCaller.cpp
        Private/Timer.cpp
        Private/Transforms.cpp
    PUBLIC
        Public/AMAssert.h
        Public/AssetCache.h
        Public/BinaryBuffer.h
        Public/ByteTools.h
        Public/ConstexprTools.h
        Public/Deserialize.h
        Public/IDPool.h
        Public/OSEventHandler.h
        Public/Ignore.h
        Public/Log.h
        Public/Paths.h
        Public/PeriodicCaller.h
        Public/Serialize.h
        Public/SerializeBuffer.h
        Public/Timer.h
        Public/Transforms.h
)

target_include_directories(Shared
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

# Build our precompiled header.
target_precompile_headers(Shared PRIVATE Private/pch.h)
