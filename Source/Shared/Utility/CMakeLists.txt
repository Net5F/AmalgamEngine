target_sources(Shared
    PRIVATE
        Private/Log.cpp
        Private/Paths.cpp
        Private/PeriodicCaller.cpp
	    Private/AssetCache.cpp
        Private/Timer.cpp
        Private/Transforms.cpp
    PUBLIC
        Public/EventHandler.h
        Public/Ignore.h
        Public/Log.h
        Public/Paths.h
        Public/PeriodicCaller.h
        Public/Profiler.h
        Public/AssetCache.h
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
