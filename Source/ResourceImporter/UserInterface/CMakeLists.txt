target_sources(ResourceImporter
    PRIVATE
        Private/Widgets/AnimationTimeline.cpp
        Private/Widgets/BoundingBoxGizmo.cpp
        Private/Widgets/GraphicSetSlot.cpp
        Private/Widgets/LibraryCollapsibleContainer.cpp
        Private/Widgets/LibraryListItem.cpp
        Private/Widgets/MainButton.cpp
        Private/Widgets/MainTextInput.cpp
        Private/Widgets/MainThumbnail.cpp
        Private/Widgets/ParentListItem.cpp
        Private/Widgets/TimelineFrame.cpp
        Private/Widgets/TimelineScrubber.cpp
        Private/Widgets/TitleButton.cpp
        Private/Windows/AddIconSheetDialog.cpp
        Private/Windows/AddSpriteSheetDialog.cpp
        Private/Windows/AnimationEditStage.cpp
        Private/Windows/AnimationPropertiesWindow.cpp
        Private/Windows/BoundingBoxEditStage.cpp
        Private/Windows/BoundingBoxPropertiesWindow.cpp
        Private/Windows/EntityGraphicSetEditStage.cpp
        Private/Windows/EntityGraphicSetPropertiesWindow.cpp
        Private/Windows/GraphicSetEditStage.cpp
        Private/Windows/GraphicSetPropertiesWindow.cpp
        Private/Windows/IconEditStage.cpp
        Private/Windows/IconPropertiesWindow.cpp
        Private/Windows/LibraryAddMenu.cpp
        Private/Windows/LibraryWindow.cpp
        Private/Windows/SaveButtonWindow.cpp
        Private/Windows/SpriteEditStage.cpp
        Private/Windows/SpritePropertiesWindow.cpp
        Private/Windows/TitleWindow.cpp
        Private/MainScreen.cpp
        Private/TitleScreen.cpp
        Private/UserInterface.cpp
    PUBLIC
        Public/Widgets/AnimationTimeline.h
        Public/Widgets/BoundingBoxGizmo.h
        Public/Widgets/GraphicSetSlot.h
        Public/Widgets/LibraryCollapsibleContainer.h
        Public/Widgets/LibraryListItem.h
        Public/Widgets/MainButton.h
        Public/Widgets/MainTextInput.h
        Public/Widgets/MainThumbnail.h
        Public/Widgets/ParentListItem.h
        Public/Widgets/TimelineFrame.h
        Public/Widgets/TimelineScrubber.h
        Public/Widgets/TitleButton.h
        Public/Windows/AddIconSheetDialog.h
        Public/Windows/AddSpriteSheetDialog.h
        Public/Windows/AnimationEditStage.h
        Public/Windows/AnimationPropertiesWindow.h
        Public/Windows/BoundingBoxEditStage.h
        Public/Windows/BoundingBoxPropertiesWindow.h
        Public/Windows/EntityGraphicSetEditStage.h
        Public/Windows/EntityGraphicSetPropertiesWindow.h
        Public/Windows/GraphicSetEditStage.h
        Public/Windows/GraphicSetPropertiesWindow.h
        Public/Windows/IconEditStage.h
        Public/Windows/IconPropertiesWindow.h
        Public/Windows/LibraryAddMenu.h
        Public/Windows/LibraryWindow.h
        Public/Windows/SaveButtonWindow.h
        Public/Windows/SpriteEditStage.h
        Public/Windows/SpritePropertiesWindow.h
        Public/Windows/TitleWindow.h
        Public/MainScreen.h
        Public/TitleScreen.h
        Public/UserInterface.h
)

target_include_directories(ResourceImporter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Private/Widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/Private/Windows
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
        ${CMAKE_CURRENT_SOURCE_DIR}/Public/Widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/Public/Windows
)
