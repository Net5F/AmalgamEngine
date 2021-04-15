#pragma once

#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/Button.h"

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
    AUI::Image background;

    AUI::Text text;

    AUI::Button loadButton;
};

} // End namespace SpriteEditor
} // End namespace AM
