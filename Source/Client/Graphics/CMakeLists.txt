target_sources(Client
	PRIVATE
		Private/Sprite.cpp
		Public/Sprite.h
		Private/AssetManager.cpp
		Public/AssetManager.h
		Private/RenderManager.cpp
		Public/RenderManager.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
