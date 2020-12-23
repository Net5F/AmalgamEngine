target_sources(Shared
    PUBLIC
        Public/Input.h
        Public/Movement.h
        Public/Name.h
        Public/Position.h
        Public/PreviousPosition.h
        Public/Sprite.h
)

target_include_directories(Shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
