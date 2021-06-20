#pragma once

#include "AUI/Screen.h"
#include "SpriteSheetPanel.h"
#include "TitleButton.h"
#include <string>

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
    MainScreen(UserInterface& inUserInterface);

    /**
     * Parses the given sprite file, loading its data into the UI.
     *
     * If the file fails to parse, tells userInterface to go back to the title
     * screen.
     */
    void loadSpriteFile(const std::string& filePath);

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    UserInterface& userInterface;

    /** The path to the sprite file that we're currently working on. */
    std::string currentSpriteFilePath;

    SpriteSheetPanel spritesheetPanel;
};

} // End namespace SpriteEditor
} // End namespace AM
