target_sources(ClientLib
    PRIVATE
        Private/UserInterface.cpp
    PUBLIC
        Public/IUserInterfaceExtension.h
        Public/PhantomTileSpriteInfo.h
        Public/TileSpriteColorModInfo.h
        Public/UserInterface.h
        Public/UserInterfaceExDependencies.h
)

target_include_directories(ClientLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
