target_sources(Client
	PRIVATE
		Private/NetworkClient.cpp
	PUBLIC
		Public/NetworkClient.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
