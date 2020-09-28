target_sources(Shared
    PRIVATE
        Private/Debug.cpp
        Private/Timer.cpp
	PUBLIC
	    Public/Debug.h
	    Public/Ignore.h
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
