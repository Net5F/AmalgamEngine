target_sources(Client
	PRIVATE
		Private/Renderer.cpp
		Private/IsoHelpers.cpp
	PUBLIC
		Public/Renderer.h
		Public/IsoHelpers.h
		Public/ScreenPoint.h
		Public/ScreenRect.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
