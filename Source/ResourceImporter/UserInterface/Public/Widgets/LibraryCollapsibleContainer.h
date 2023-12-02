#pragma once

#include "AUI/CollapsibleContainer.h"

namespace AM
{
namespace ResourceImporter
{
/**
 * A collapsible container used for the top-level categories in the library 
 * window on the main screen.
 */
class LibraryCollapsibleContainer : public AUI::CollapsibleContainer
{
public:
    LibraryCollapsibleContainer(const std::string& inHeaderText,
                      const std::string& inDebugName
                         = "LibraryCollapsibleContainer");

    /**
     * Sets the left padding. Used to define the visual hierarchy in the list.
     */
    void setLeftPadding(int inLeftPadding);
};

} // End namespace ResourceImporter
} // End namespace AM
