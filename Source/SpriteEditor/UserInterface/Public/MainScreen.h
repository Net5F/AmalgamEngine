#pragma once

#include "SpriteDataModel.h"
#include "AUI/Screen.h"
#include "SpriteSheetPanel.h"
#include "TitleButton.h"
#include <filesystem>

namespace AM
{
namespace SpriteEditor
{

class UserInterface;

/**
 * The main screen for doing work.
 */
class MainScreen : public AUI::Screen
{
public:
    MainScreen(SpriteDataModel& inSpriteDataModel);

    /**
     * Loads the current state of spriteData into this screen's UI.
     */
    void loadSpriteData();

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    /** The sprite data for this project. Used by this screen's UI. */
    SpriteDataModel& spriteDataModel;

    SpriteSheetPanel spritesheetPanel;
};

} // End namespace SpriteEditor
} // End namespace AM
