target_sources(AccountServer
    PRIVATE
        Private/DataModel.cpp
    PUBLIC
        Public/DataModel.h
)

target_include_directories(AccountServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
