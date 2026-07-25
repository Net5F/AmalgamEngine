target_sources(AccountLib
    PRIVATE
        Private/Dummy.cpp
    PUBLIC
        Public/AccountDefs.h
)

target_include_directories(AccountLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
