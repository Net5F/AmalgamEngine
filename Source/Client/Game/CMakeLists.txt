target_sources(Client
	PRIVATE
		Private/World.cpp
		Private/PlayerInputSystem.cpp
		Private/MovementSystem.cpp
		Private/RenderSystem.cpp
	PUBLIC
		Public/World.h
		Public/PlayerInputSystem.h
		Public/MovementSystem.h
		Public/RenderSystem.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
