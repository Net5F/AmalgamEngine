target_sources(Shared
    PUBLIC
        Public/InputComponent.h
        Public/MovementComponent.h
        Public/PositionComponent.h
        Public/SpriteComponent.h
)

target_include_directories(Shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
