target_sources(Client
    PRIVATE
        Private/ClientMovementHelpers.cpp
        Private/ClientTransforms.cpp
    PUBLIC
        Public/ClientMovementHelpers.h
        Public/ClientTransforms.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
