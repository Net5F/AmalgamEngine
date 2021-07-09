#include "MainScreen.h"
#include "UserInterface.h"
#include "AUI/Core.h"
#include "nfd.h"
#include "Log.h"

namespace AM
{
namespace SpriteEditor
{

MainScreen::MainScreen(SpriteDataModel& inSpriteDataModel)
: Screen("MainScreen")
, spriteDataModel{inSpriteDataModel}
, activeSprite{nullptr}
, spriteSheetPanel(*this, spriteDataModel)
, spriteEditStage(*this)
, spritePanel(*this)
, saveButton(*this, "", {1537, 0, 58, 58})
, propertiesPanel(*this)
, dialogShadowImage(*this, "", {0, 0, 1920, 1080})
, confirmationDialog(*this, "", {721, 358, 474, 248})
{
    /* Save button. */
    saveButton.normalImage.addResolution({1920, 1080}, "Textures/SaveButton/Normal.png");
    saveButton.hoveredImage.addResolution({1920, 1080}, "Textures/SaveButton/Hovered.png");
    saveButton.pressedImage.addResolution({1920, 1080}, "Textures/SaveButton/Pressed.png");
    saveButton.text.setFont("Fonts/B612-Regular.ttf", 33);
    saveButton.text.setText("");

    // Add a callback to save the current sprite data when pressed.
    saveButton.setOnPressed([this]() {
        // Create our callback.
        std::function<void(void)> onConfirmation = [&]() {
            // Save the data.
            spriteDataModel.save();
        };

        // Open the confirmation dialog.
        openConfirmationDialog("Save over existing SpriteData.json?", "SAVE"
                               , std::move(onConfirmation));
    });

    /* Confirmation dialog. */
    // Background shadow image.
    dialogShadowImage.addResolution({1920, 1080}, "Textures/Dialogs/Shadow.png");

    // Background image.
    confirmationDialog.backgroundImage.setLogicalExtent({0, 0, 474, 248});
    confirmationDialog.backgroundImage.addResolution({1920, 1080}, "Textures/Dialogs/Background.png");

    // Body text.
    confirmationDialog.bodyText.setLogicalExtent({42, 42, 400, 60});
    confirmationDialog.bodyText.setFont("Fonts/B612-Regular.ttf", 21);
    confirmationDialog.bodyText.setColor({255, 255, 255, 255});

    // Buttons.
    confirmationDialog.confirmButton.setLogicalExtent({324, 162, 123, 56});
    confirmationDialog.confirmButton.normalImage.setLogicalExtent({0, 0, 123, 56});
    confirmationDialog.confirmButton.hoveredImage.setLogicalExtent({0, 0, 123, 56});
    confirmationDialog.confirmButton.pressedImage.setLogicalExtent({0, 0, 123, 56});
    confirmationDialog.confirmButton.text.setLogicalExtent({-1, -1, 123, 56});
    confirmationDialog.confirmButton.normalImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Normal.png");
    confirmationDialog.confirmButton.hoveredImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Hovered.png");
    confirmationDialog.confirmButton.pressedImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Pressed.png");
    confirmationDialog.confirmButton.text.setFont("Fonts/B612-Regular.ttf", 18);
    confirmationDialog.confirmButton.text.setColor({255, 255, 255, 255});

    confirmationDialog.cancelButton.setLogicalExtent({182, 162, 123, 56});
    confirmationDialog.cancelButton.normalImage.setLogicalExtent({0, 0, 123, 56});
    confirmationDialog.cancelButton.hoveredImage.setLogicalExtent({0, 0, 123, 56});
    confirmationDialog.cancelButton.pressedImage.setLogicalExtent({0, 0, 123, 56});
    confirmationDialog.cancelButton.text.setLogicalExtent({-1, -1, 123, 56});
    confirmationDialog.cancelButton.normalImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Normal.png");
    confirmationDialog.cancelButton.hoveredImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Hovered.png");
    confirmationDialog.cancelButton.pressedImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Pressed.png");
    confirmationDialog.cancelButton.text.setFont("Fonts/B612-Regular.ttf", 18);
    confirmationDialog.cancelButton.text.setColor({255, 255, 255, 255});
    confirmationDialog.cancelButton.text.setText("CANCEL");

    // Set up the dialog's cancel button callback.
    confirmationDialog.cancelButton.setOnPressed([this]() {
        // Close the dialog.
        dialogShadowImage.setIsVisible(false);
        confirmationDialog.setIsVisible(false);
    });

    // Make the confirmationDialog invisible. Components that want to use it can set it up
    // and control the visibility.
    dialogShadowImage.setIsVisible(false);
    confirmationDialog.setIsVisible(false);
}

void MainScreen::loadSpriteData()
{
    // Clear out the old components.
    spriteSheetPanel.clearSpriteSheets();
    spritePanel.clearSprites();
    propertiesPanel.clear();
    activeSprite = nullptr;

    // Load the model's data into this screen's UI.
    // For each sprite sheet in the model.
    for (SpriteSheet& sheet : spriteDataModel.getSpriteSheets()) {
        // Add a Thumbnail component that displays the sheet.
        spriteSheetPanel.addSpriteSheet(sheet);

        // For each sprite in the sheet.
        for (SpriteStaticData& sprite : sheet.sprites) {
            // Add a Thumbnail component that displays the sprite.
            spritePanel.addSprite(sheet, sprite);
        }
    }
}

void MainScreen::openConfirmationDialog(const std::string& bodyText
                                , const std::string& confirmButtonText
                                , std::function<void(void)> onConfirmation)
{
    // Set the dialog's text.
    confirmationDialog.bodyText.setText(bodyText);
    confirmationDialog.confirmButton.text.setText(confirmButtonText);

    // Set the dialog's confirmation callback.
    userOnConfirmation = std::move(onConfirmation);
    confirmationDialog.confirmButton.setOnPressed([&]() {
        // Call the user's callback.
        userOnConfirmation();

        // Close the dialog.
        dialogShadowImage.setIsVisible(false);
        confirmationDialog.setIsVisible(false);
    });

    // Open the dialog.
    dialogShadowImage.setIsVisible(true);
    confirmationDialog.setIsVisible(true);
}

void MainScreen::setActiveSprite(SpriteStaticData* inActiveSprite)
{
    // Save the sprite pointer.
    activeSprite = inActiveSprite;

    // Load the sprite's data into the properties panel.
    propertiesPanel.loadActiveSprite();

    // Load the sprite onto the stage.
    spriteEditStage.loadActiveSprite();
}

SpriteStaticData* MainScreen::getActiveSprite()
{
    return activeSprite;
}

void MainScreen::render()
{
    // Fill the background with the background color.
    SDL_Renderer* renderer = AUI::Core::GetRenderer();
    SDL_SetRenderDrawColor(renderer, 17, 17, 19, 255);
    SDL_RenderClear(renderer);

    // Render our children.
    spriteEditStage.render();

    spriteSheetPanel.render();

    spritePanel.render();

    saveButton.render();

    propertiesPanel.render();

    // Render the confirmation dialog on top of everything else.
    dialogShadowImage.render();
    confirmationDialog.render();
}

} // End namespace SpriteEditor
} // End namespace AM
