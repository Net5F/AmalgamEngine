target_sources(ClientLib
	PRIVATE
		Private/Network.cpp
		Private/MessageProcessor.cpp
	PUBLIC
		Public/ConnectionError.h
		Public/IMessageProcessorExtension.h
		Public/MessageProcessor.h
		Public/MessageProcessorExDependencies.h
		Public/Network.h
		Public/PlayerMovementUpdate.h
)

target_include_directories(ClientLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
