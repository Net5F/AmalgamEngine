#pragma once

#include "AUI/Screen.h"
#include "TitleWindow.h"

namespace AM
{
namespace SpriteEditor
{
class UserInterface;
class SpriteDataModel;

/**
 * The opening title screen that you see on app launch.
 */
class TitleScreen : public AUI::Screen
{
public:
    TitleScreen(UserInterface& inUserInterface,
                SpriteDataModel& inSpriteDataModel);

    void render() override;

private:
    //-------------------------------------------------------------------------
    // Windows
    //-------------------------------------------------------------------------
    TitleWindow titleWindow;
};

} // End namespace SpriteEditor
} // End namespace AM
