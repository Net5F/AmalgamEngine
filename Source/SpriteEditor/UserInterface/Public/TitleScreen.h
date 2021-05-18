#pragma once

#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "TitleButton.h"

namespace AM
{
namespace SpriteEditor
{

/**
 * The opening title screen that you see on app launch.
 */
class TitleScreen : public AUI::Screen
{
public:
    TitleScreen();

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    AUI::Image background;

    TitleButton newButton;

    TitleButton loadButton;
};

} // End namespace SpriteEditor
} // End namespace AM
