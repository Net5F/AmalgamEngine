target_sources(SpriteEditor
	PRIVATE
		Private/TitleScreen.cpp
		Private/UserInterface.cpp
	PUBLIC
		Public/TitleScreen.h
		Public/UserInterface.h
)

target_include_directories(SpriteEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
