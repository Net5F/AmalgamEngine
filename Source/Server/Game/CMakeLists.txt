target_sources(Server
	PRIVATE
		Private/Network.cpp
		Private/World.cpp
		Private/MovementSystem.cpp
		Private/NetworkInputSystem.cpp
	PUBLIC
		Public/Network.h
		Public/World.h
		Public/MovementSystem.h
		Public/NetworkInputSystem.h
)

target_include_directories(Server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
