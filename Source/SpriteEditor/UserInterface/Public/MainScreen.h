#pragma once

#include "SpriteDataModel.h"
#include "AUI/Screen.h"
#include "AUI/Button.h"
#include "AUI/ConfirmationDialog.h"
#include "SpriteSheetPanel.h"
#include "SpritePanel.h"
#include "TitleButton.h"

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

    /**
     * Opens a confirmation dialog.
     *
     * @param bodyText  The main dialog text.
     * @param confirmButtonText  The text on the confirm button.
     * @param onConfirmation  Called when the user presses the confirm button.
     */
    void openConfirmationDialog(const std::string& bodyText
                                , const std::string& confirmButtonText
                                , std::function<void(void)> onConfirmation);

    void setActiveSprite(const SpriteStaticData* inActiveSprite);
    const SpriteStaticData* getActiveSprite();

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    /** The sprite data for this project. Used by this screen's UI. */
    SpriteDataModel& spriteDataModel;

    /** If non-nullptr, this is the currently active sprite's data. The active
        sprite's data is shown in the properties panel, and holds the extent
        of the AABB control gizmo. */
    const SpriteStaticData* activeSprite;

    /** The left-side panel for managing sprite texture sheets. */
    SpriteSheetPanel spriteSheetPanel;

    /** The bottom panel for selecting sprites. */
    SpritePanel spritePanel;

    /** The save button at the top of the screen, next to the properties. */
    AUI::Button saveButton;

    /** Background semi-transparent shadow image to go behind the confirmation
        dialog. */
    AUI::Image dialogShadowImage;

    /** Confirmation dialog. Child components can call openConfirmationDialog()
        to use it. */
    AUI::ConfirmationDialog confirmationDialog;

    /** The confirmationDialog user's callback. Set while opening the dialog. */
    std::function<void(void)> userOnConfirmation;
};

} // End namespace SpriteEditor
} // End namespace AM
