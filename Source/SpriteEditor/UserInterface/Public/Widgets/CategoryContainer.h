#pragma once

#include "AUI/CollapsibleContainer.h"

namespace AM
{
namespace SpriteEditor
{
/**
 * A collapsible container used for the top-level categories in the library 
 * window on the main screen.
 */
class CategoryContainer : public AUI::CollapsibleContainer
{
public:
    CategoryContainer(const std::string& inHeaderText,
                      const std::string& inDebugName
                         = "CategoryContainer");

    /**
     * Sets the left padding. Used to define the visual hierarchy in the list.
     */
    void setLeftPadding(int inLeftPadding);
};

} // End namespace SpriteEditor
} // End namespace AM
