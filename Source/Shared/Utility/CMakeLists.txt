target_sources(Shared
    PRIVATE
        Private/Log.cpp
        Private/PeriodicCaller.cpp
        Private/Timer.cpp
	PUBLIC
	    Public/Log.h
	    Public/Ignore.h
	    Public/PeriodicCaller.h
	    Public/Profiler.h
	    Public/Timer.h
)

target_include_directories(Shared
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

# Build our precompiled header.
target_precompile_headers(Shared PRIVATE Private/pch.h)
