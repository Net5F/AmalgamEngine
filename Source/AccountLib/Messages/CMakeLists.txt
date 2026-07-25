target_sources(AccountLib
    PUBLIC
        Public/AccountMessageType.h
        Public/AccountRegisterRequest.h
        Public/AccountRegisterResponse.h
)

target_include_directories(AccountLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
