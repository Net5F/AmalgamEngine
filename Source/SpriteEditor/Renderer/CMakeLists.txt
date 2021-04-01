target_sources(SpriteEditor
	PRIVATE
		Private/Renderer.cpp
	PUBLIC
		Public/Renderer.h
)

target_include_directories(SpriteEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
