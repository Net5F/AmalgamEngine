target_sources(Client
	PRIVATE
		Private/Renderer.cpp
		Private/WorldSpritePreparer.cpp
	PUBLIC
		Public/Renderer.h
		Public/ScreenPoint.h
		Public/ScreenRect.h
		Public/WorldSpritePreparer.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
