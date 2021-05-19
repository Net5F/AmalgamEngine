#pragma once

#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "TitleButton.h"

namespace AM
{
namespace SpriteEditor
{

class UserInterface;

/**
 * The opening title screen that you see on app launch.
 */
class TitleScreen : public AUI::Screen
{
public:
    TitleScreen(UserInterface& inUserInterface);

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    UserInterface& userInterface;

    AUI::Image background;

    TitleButton newButton;

    TitleButton loadButton;

    AUI::Text errorText;
};

} // End namespace SpriteEditor
} // End namespace AM
