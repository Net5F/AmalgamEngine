target_sources(Shared
    PRIVATE
        Private/AssetCache.cpp
        Private/ByteTools.cpp
        Private/Log.cpp
        Private/Paths.cpp
        Private/PeriodicCaller.cpp
        Private/Timer.cpp
        Private/Transforms.cpp
    PUBLIC
        Public/AssetCache.h
        Public/BinaryBuffer.h
        Public/ByteTools.h
        Public/Deserialize.h
        Public/OSEventHandler.h
        Public/Ignore.h
        Public/Log.h
        Public/Paths.h
        Public/PeriodicCaller.h
        Public/Profiler.h
        Public/Serialize.h
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
