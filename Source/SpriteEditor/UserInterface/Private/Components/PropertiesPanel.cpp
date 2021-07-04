#include "PropertiesPanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Ignore.h"

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
{
    /* Background image */
    backgroundImage.addResolution({1920, 1080}, "Textures/PropertiesPanel/Background.png");

    /* Name entry. */
    nameLabel.setFont("Fonts/B612-Regular.ttf", 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    nameInput.setMargins({8, 0, 8, 0});

    /* Min X entry. */
    minXLabel.setFont("Fonts/B612-Regular.ttf", 21);
    minXLabel.setColor({255, 255, 255, 255});
    minXLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minXLabel.setText("Min X");

    minXInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    minXInput.setMargins({8, 0, 8, 0});

    /* Min Y entry. */
    minYLabel.setFont("Fonts/B612-Regular.ttf", 21);
    minYLabel.setColor({255, 255, 255, 255});
    minYLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minYLabel.setText("Min Y");

    minYInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    minYInput.setMargins({8, 0, 8, 0});

    /* Min Z entry. */
    minZLabel.setFont("Fonts/B612-Regular.ttf", 21);
    minZLabel.setColor({255, 255, 255, 255});
    minZLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minZLabel.setText("Min Z");

    minZInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    minZInput.setMargins({8, 0, 8, 0});

    /* Max X entry. */
    maxXLabel.setFont("Fonts/B612-Regular.ttf", 21);
    maxXLabel.setColor({255, 255, 255, 255});
    maxXLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxXLabel.setText("Max X");

    maxXInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    maxXInput.setMargins({8, 0, 8, 0});

    /* Max Y entry. */
    maxYLabel.setFont("Fonts/B612-Regular.ttf", 21);
    maxYLabel.setColor({255, 255, 255, 255});
    maxYLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxYLabel.setText("Max Y");

    maxYInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    maxYInput.setMargins({8, 0, 8, 0});

    /* Max Z entry. */
    maxZLabel.setFont("Fonts/B612-Regular.ttf", 21);
    maxZLabel.setColor({255, 255, 255, 255});
    maxZLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxZLabel.setText("Max Z");

    maxZInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    maxZInput.setMargins({8, 0, 8, 0});
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

void PropertiesPanel::clearTextInputs()
{
    nameInput.setText("");
    minXInput.setText("");
    minYInput.setText("");
    minZInput.setText("");
    maxXInput.setText("");
    maxYInput.setText("");
    maxZInput.setText("");
}

} // End namespace SpriteEditor
} // End namespace AM
