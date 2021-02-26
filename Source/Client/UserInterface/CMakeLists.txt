target_sources(Client
	PRIVATE
		Private/UserInterface.cpp
	PUBLIC
		Public/UserInterface.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
