target_sources(Client
	PRIVATE
		Private/Network.cpp
	PUBLIC
		Public/Network.h
		Public/ClientNetworkDefs.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
