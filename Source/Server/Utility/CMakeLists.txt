target_sources(Server
	PRIVATE
		Private/IDPool.cpp
		Private/NetworkServer.cpp
	PUBLIC
		Public/IDPool.h
		Public/NetworkServer.h
)

target_include_directories(Server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
