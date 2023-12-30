#include "LibraryCollapsibleContainer.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
LibraryCollapsibleContainer::LibraryCollapsibleContainer(
    const std::string& inHeaderText, const std::string& inDebugName)
: AUI::CollapsibleContainer({0, 0, 318, 30}, inDebugName)
{
    // Add our arrow images.
    expandedImage.setLogicalExtent({8, 4, 20, 20});
    collapsedImage.setLogicalExtent({8, 4, 20, 20});
    expandedImage.setMultiResImage(
        {{{1600, 900},
          (Paths::TEXTURE_DIR + "LibraryWindow/ExpandedIcon_1600.png")},
         {{1920, 1080},
          (Paths::TEXTURE_DIR + "LibraryWindow/ExpandedIcon_1920.png")}});
    collapsedImage.setMultiResImage(
        {{{1600, 900},
          (Paths::TEXTURE_DIR + "LibraryWindow/CollapsedIcon_1600.png")},
         {{1920, 1080},
          (Paths::TEXTURE_DIR + "LibraryWindow/CollapsedIcon_1920.png")}});

    // Set our text properties.
    headerText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    headerText.setColor({255, 255, 255, 255});
    headerText.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    headerText.setText(inHeaderText);
    headerText.setLogicalExtent(
        {25, 0, (logicalExtent.w - 25), logicalExtent.h});

    // Set our padding.
    setLeftPadding(8);
}

void LibraryCollapsibleContainer::setLeftPadding(int inLeftPadding)
{
    int imageX{0 + inLeftPadding};
    int textX{25 + inLeftPadding};

    expandedImage.setLogicalExtent({imageX, 4, 20, 20});
    collapsedImage.setLogicalExtent({imageX, 4, 20, 20});
    headerText.setLogicalExtent(
        {textX, 0, (logicalExtent.w - textX), logicalExtent.h});
}

} // End namespace ResourceImporter
} // End namespace AM
