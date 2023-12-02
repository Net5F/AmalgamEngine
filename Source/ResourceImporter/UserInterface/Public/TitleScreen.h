#pragma once

#include "AUI/Screen.h"
#include "TitleWindow.h"

namespace AM
{
namespace ResourceImporter
{
class UserInterface;
class DataModel;

/**
 * The opening title screen that you see on app launch.
 */
class TitleScreen : public AUI::Screen
{
public:
    TitleScreen(UserInterface& inUserInterface, DataModel& inDataModel);

    void render() override;

private:
    //-------------------------------------------------------------------------
    // Windows
    //-------------------------------------------------------------------------
    TitleWindow titleWindow;
};

} // End namespace ResourceImporter
} // End namespace AM
