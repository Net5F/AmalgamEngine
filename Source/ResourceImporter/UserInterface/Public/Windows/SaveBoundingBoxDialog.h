#pragma once

#include "BoundingBoxID.h"
#include "BoundingBox.h"
#include "MainButton.h"
#include "MainTextInput.h"
#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include <functional>

namespace AM
{
namespace ResourceImporter
{
class DataModel;

/**
 * A confirmation dialog for saving BoundingBoxes to the Library.
 *
 * Opened by using the "Save as" button in the Sprite or Animation properties 
 * panels.
 */
class SaveBoundingBoxDialog : public AUI::Window
{
public:
    SaveBoundingBoxDialog(DataModel& inDataModel);

    virtual ~SaveBoundingBoxDialog() = default;

    /**
     * Sets the model bounds that should be saved when the "Save" button is 
     * pressed, and the callback that should be called afterwards.
     */
    void setSaveData(const BoundingBox& inModelBoundsToSave,
                     const std::function<void(BoundingBoxID)> inSaveCallback);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Semi-transparent shadow image to obscure things that are behind the
        dialog. */
    AUI::Image shadowImage;

    /** The dialog's background image. */
    AUI::Image backgroundImage;

    /** The header text at the top of the dialog. */
    AUI::Text headerText;

    // Bounding box name entry
    AUI::Text nameLabel;
    MainTextInput nameInput;

    /** Text that warns the user about overwriting data. */
    AUI::Text descriptionText;

    /** Right-side confirmation button. Saves the bounding box. */
    MainButton saveButton;

    /** Left-side cancel button. Closes the dialog without performing any
        action. */
    MainButton cancelButton;

private:
    /**
     * Clears the text in all of this dialog's text inputs and error text.
     */
    void clear();

    /** Used to update the model when a bounding box is saved. */
    DataModel& dataModel;

    /** The model bounds to save when the "Save" button is pressed. */
    BoundingBox modelBoundsToSave;

    /** The callback that should be called after the bounding box is saved. */
    std::function<void(BoundingBoxID)> saveCallback;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    /** Error text, appears if the user tries to submit an invalid input. */
    AUI::Text errorText;
};

} // End namespace ResourceImporter
} // End namespace AM
