target_sources(Server
	PRIVATE
		Private/IDPool.cpp
		Private/Network.cpp
	PUBLIC
		Public/IDPool.h
		Public/Network.h
)

target_include_directories(Server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
