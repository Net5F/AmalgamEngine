target_sources(Shared
    PRIVATE
        Private/MovementHelpers.cpp
	PUBLIC
        Public/GameDefs.h
        Public/MovementHelpers.h
)

target_include_directories(Shared
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
