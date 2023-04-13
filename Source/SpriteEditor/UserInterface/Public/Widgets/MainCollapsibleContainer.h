#pragma once

#include "AUI/CollapsibleContainer.h"

namespace AM
{
namespace SpriteEditor
{
/**
 * A collapsible container used for the library window on the main screen.
 */
class MainCollapsibleContainer : public AUI::CollapsibleContainer
{
public:
    MainCollapsibleContainer(const std::string& inHeaderText,
                             const std::string& inDebugName
                             = "MainCollapsibleContainer");

    /**
     * Sets the left padding. Used to define the visual hierarchy in the list.
     */
    void setLeftPadding(int inLeftPadding);
};

} // End namespace SpriteEditor
} // End namespace AM
