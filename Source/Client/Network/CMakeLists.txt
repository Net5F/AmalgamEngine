target_sources(Client
	PRIVATE
		Private/Network.cpp
		Private/MessageProcessor.cpp
	PUBLIC
		Public/Network.h
		Public/MessageProcessor.h
		Public/ClientNetworkDefs.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
