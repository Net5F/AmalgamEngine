#include "PropertiesPanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Ignore.h"
#include <string>

namespace AM
{
namespace SpriteEditor
{

PropertiesPanel::PropertiesPanel(MainScreen& inScreen)
: AUI::Component(inScreen, "PropertiesPanel", {1617, 0, 303, 440})
, nameLabel(inScreen, "", {24, 24, 65, 28})
, nameInput(inScreen, "", {24, 56, 255, 38})
, minXLabel(inScreen, "", {24, 126, 110, 38})
, minXInput(inScreen, "", {150, 126, 129, 38})
, minYLabel(inScreen, "", {24, 176, 110, 38})
, minYInput(inScreen, "", {150, 176, 129, 38})
, minZLabel(inScreen, "", {24, 226, 110, 38})
, minZInput(inScreen, "", {150, 226, 129, 38})
, maxXLabel(inScreen, "", {24, 276, 110, 38})
, maxXInput(inScreen, "", {150, 276, 129, 38})
, maxYLabel(inScreen, "", {24, 326, 110, 38})
, maxYInput(inScreen, "", {150, 326, 129, 38})
, maxZLabel(inScreen, "", {24, 376, 110, 38})
, maxZInput(inScreen, "", {150, 376, 129, 38})
, mainScreen{inScreen}
, backgroundImage(inScreen, "", {-12, -4, 319, 456})
, committedMinX{0.0}
, committedMinY{0.0}
, committedMinZ{0.0}
, committedMaxX{0.0}
, committedMaxY{0.0}
, committedMaxZ{0.0}
{
    /* Background image */
    backgroundImage.addResolution({1920, 1080}, "Textures/PropertiesPanel/Background.png");

    /* Display name entry. */
    nameLabel.setFont("Fonts/B612-Regular.ttf", 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    nameInput.setMargins({8, 0, 8, 0});
    nameInput.setOnTextCommitted([this]() {
        saveName();
    });

    /* Minimum X-axis bounds entry. */
    minXLabel.setFont("Fonts/B612-Regular.ttf", 21);
    minXLabel.setColor({255, 255, 255, 255});
    minXLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minXLabel.setText("Min X");

    minXInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    minXInput.setMargins({8, 0, 8, 0});
    minXInput.setOnTextCommitted([this]() {
        saveMinX();
    });

    /* Minimum Y-axis bounds entry. */
    minYLabel.setFont("Fonts/B612-Regular.ttf", 21);
    minYLabel.setColor({255, 255, 255, 255});
    minYLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minYLabel.setText("Min Y");

    minYInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    minYInput.setMargins({8, 0, 8, 0});
    minYInput.setOnTextCommitted([this]() {
        saveMinY();
    });

    /* Minimum Z-axis bounds entry. */
    minZLabel.setFont("Fonts/B612-Regular.ttf", 21);
    minZLabel.setColor({255, 255, 255, 255});
    minZLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minZLabel.setText("Min Z");

    minZInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    minZInput.setMargins({8, 0, 8, 0});
    minZInput.setOnTextCommitted([this]() {
        saveMinZ();
    });

    /* Maximum X-axis bounds entry. */
    maxXLabel.setFont("Fonts/B612-Regular.ttf", 21);
    maxXLabel.setColor({255, 255, 255, 255});
    maxXLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxXLabel.setText("Max X");

    maxXInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    maxXInput.setMargins({8, 0, 8, 0});
    maxXInput.setOnTextCommitted([this]() {
        saveMaxX();
    });

    /* Maximum Y-axis bounds entry. */
    maxYLabel.setFont("Fonts/B612-Regular.ttf", 21);
    maxYLabel.setColor({255, 255, 255, 255});
    maxYLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxYLabel.setText("Max Y");

    maxYInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    maxYInput.setMargins({8, 0, 8, 0});
    maxYInput.setOnTextCommitted([this]() {
        saveMaxY();
    });

    /* Maximum Z-axis bounds entry. */
    maxZLabel.setFont("Fonts/B612-Regular.ttf", 21);
    maxZLabel.setColor({255, 255, 255, 255});
    maxZLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxZLabel.setText("Max Z");

    maxZInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    maxZInput.setMargins({8, 0, 8, 0});
    maxZInput.setOnTextCommitted([this]() {
        saveMaxZ();
    });
}

void PropertiesPanel::loadSprite(const SpriteStaticData& sprite)
{
    // Fill the fields with the sprite data.
    nameInput.setText(sprite.displayName);
    minXInput.setText(std::to_string(sprite.modelBounds.minX));
    minYInput.setText(std::to_string(sprite.modelBounds.minY));
    minZInput.setText(std::to_string(sprite.modelBounds.minZ));
    maxXInput.setText(std::to_string(sprite.modelBounds.maxX));
    maxYInput.setText(std::to_string(sprite.modelBounds.maxY));
    maxZInput.setText(std::to_string(sprite.modelBounds.maxZ));
}

void PropertiesPanel::clear()
{
    nameInput.setText("");
    minXInput.setText("");
    minYInput.setText("");
    minZInput.setText("");
    maxXInput.setText("");
    maxYInput.setText("");
    maxZInput.setText("");
}

void PropertiesPanel::render(const SDL_Point& parentOffset)
{
    // Keep our scaling up to date.
    refreshScaling();

    // Save the extent that we're going to render at.
    lastRenderedExtent = scaledExtent;
    lastRenderedExtent.x += parentOffset.x;
    lastRenderedExtent.y += parentOffset.y;

    // Children should render at the parent's offset + this component's offset.
    SDL_Point childOffset{parentOffset};
    childOffset.x += scaledExtent.x;
    childOffset.y += scaledExtent.y;

    // Render our children.
    backgroundImage.render(childOffset);

    nameLabel.render(childOffset);
    nameInput.render(childOffset);

    minXLabel.render(childOffset);
    minXInput.render(childOffset);

    minYLabel.render(childOffset);
    minYInput.render(childOffset);

    minZLabel.render(childOffset);
    minZInput.render(childOffset);

    maxXLabel.render(childOffset);
    maxXInput.render(childOffset);

    maxYLabel.render(childOffset);
    maxYInput.render(childOffset);

    maxZLabel.render(childOffset);
    maxZInput.render(childOffset);
}

void PropertiesPanel::saveName()
{
    SpriteStaticData* sprite = mainScreen.getActiveSprite();
    if (sprite != nullptr) {
        // Save the display name.
        // Note: All characters that a user can enter are valid in the display
        //       name string, so we don't need to validate.
        sprite->displayName = nameInput.getText();

        // Re-load the UI since the name is shown in the sprite panel.
        mainScreen.loadSpriteData();
    }
}

void PropertiesPanel::saveMinX()
{
    SpriteStaticData* sprite = mainScreen.getActiveSprite();
    if (sprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMinX{std::stof(minXInput.getText())};

            // The input was valid, save it.
            sprite->modelBounds.minX = newMinX;
            committedMinX = newMinX;
        }
        catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            minXInput.setText(std::to_string(committedMinX));
        }
    }
}

void PropertiesPanel::saveMinY()
{
    SpriteStaticData* sprite = mainScreen.getActiveSprite();
    if (sprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMinY{std::stof(minYInput.getText())};

            // The input was valid, save it.
            sprite->modelBounds.minY = newMinY;
            committedMinY = newMinY;
        }
        catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            minYInput.setText(std::to_string(committedMinY));
        }
    }
}

void PropertiesPanel::saveMinZ()
{
    SpriteStaticData* sprite = mainScreen.getActiveSprite();
    if (sprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMinZ{std::stof(minZInput.getText())};

            // The input was valid, save it.
            sprite->modelBounds.minZ = newMinZ;
            committedMinZ = newMinZ;
        }
        catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            minZInput.setText(std::to_string(committedMinZ));
        }
    }
}

void PropertiesPanel::saveMaxX()
{
    SpriteStaticData* sprite = mainScreen.getActiveSprite();
    if (sprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMaxX{std::stof(maxXInput.getText())};

            // The input was valid, save it.
            sprite->modelBounds.maxX = newMaxX;
            committedMaxX = newMaxX;
        }
        catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            maxXInput.setText(std::to_string(committedMaxX));
        }
    }
}

void PropertiesPanel::saveMaxY()
{
    SpriteStaticData* sprite = mainScreen.getActiveSprite();
    if (sprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMaxY{std::stof(maxYInput.getText())};

            // The input was valid, save it.
            sprite->modelBounds.maxY = newMaxY;
            committedMaxY = newMaxY;
        }
        catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            maxYInput.setText(std::to_string(committedMaxY));
        }
    }
}

void PropertiesPanel::saveMaxZ()
{
    SpriteStaticData* sprite = mainScreen.getActiveSprite();
    if (sprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMaxZ{std::stof(maxZInput.getText())};

            // The input was valid, save it.
            sprite->modelBounds.maxZ = newMaxZ;
            committedMaxZ = newMaxZ;
        }
        catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            maxZInput.setText(std::to_string(committedMaxZ));
        }
    }
}

} // End namespace SpriteEditor
} // End namespace AM
