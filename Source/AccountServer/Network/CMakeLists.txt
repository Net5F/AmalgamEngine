target_sources(AccountServer
    PRIVATE
        Private/MessageProcessor.cpp
        Private/Network.cpp
    PUBLIC
        Public/MessageProcessor.h
        Public/Network.h
)

target_include_directories(AccountServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
