target_sources(AccountServer
    PRIVATE
        Private/Client.cpp
        Private/ClientManager.cpp
        Private/MessageProcessor.cpp
    PUBLIC
        Public/Client.h
        Public/ClientManager.h
        Public/MessageProcessor.h
)

target_include_directories(AccountServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
