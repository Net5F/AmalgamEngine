#include "SpriteSheetPanel.h"
#include "MainThumbnail.h"

namespace AM
{

SpriteSheetPanel::SpriteSheetPanel(AUI::Screen& screen)
: AUI::Component(screen, "SpriteSheetPanel", {0, 0, 399, 708})
, backgroundImage(screen, "", logicalExtent)
, spritesheetContainer(screen, "SpriteSheetContainer", {100, 100, 350, 600})
, remSheetButton(screen, "", {342, 0, 45, 63})
, addSheetButton(screen, "", {342, 63, 45, 88})
{
    /* Background image */
    backgroundImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/Background_1920.png"
                                  , {4, 4, 399, 708});
    backgroundImage.addResolution({1600, 900}, "Textures/SpriteSheetPanel/Background_1600.png"
                                  , {4, 4, 333, 590});
    backgroundImage.addResolution({1280, 720}, "Textures/SpriteSheetPanel/Background_1280.png"
                                  , {3, 3, 266, 472});

    /* Container */
    for (unsigned int i = 0; i < 3; ++i) {
        std::unique_ptr<AUI::Component> thumbnail{
            std::make_unique<MainThumbnail>(screen, "")};
        MainThumbnail& thumbRef{static_cast<MainThumbnail&>(*thumbnail)};

        thumbRef.thumbnailImage.addResolution({1280, 720}, "Textures/Temp/iso_test_sprites.png");
        thumbRef.setText("Component " + std::to_string(i));

        spritesheetContainer.push_back(std::move(thumbnail));
    }

    /* Remove sheet button */
    remSheetButton.normalImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveNormal.png");
    remSheetButton.hoveredImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveHovered.png");
    remSheetButton.pressedImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveNormal.png");
    remSheetButton.disabledImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveDisabled.png");
    remSheetButton.text.setFont("Fonts/B612-Regular.ttf", 33);
    remSheetButton.text.setText("");

    // Remove a component on button press.
    remSheetButton.setOnPressed([&](){
//        AUI::Container& spriteContainer{
//            this->get<AUI::Container>("SpritesheetContainer")};
//        AUI::Component* component = &(spriteContainer[0u]);
//        spriteContainer.erase(component);
        LOG_INFO("Hi");
    });

    /* Add sheet button */
    addSheetButton.normalImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/AddNormal.png");
    addSheetButton.hoveredImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/AddHovered.png");
    addSheetButton.pressedImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/AddNormal.png");
    addSheetButton.text.setFont("Fonts/B612-Regular.ttf", 33);
    addSheetButton.text.setText("");

    addSheetButton.setOnPressed([&](){
        LOG_INFO("Hi");
    });
}

void SpriteSheetPanel::render(const SDL_Point& parentOffset)
{
    // Keep our scaling up to date.
    refreshScaling();

    // Render our children.
    backgroundImage.render(parentOffset);

    spritesheetContainer.render(parentOffset);

    remSheetButton.render(parentOffset);

    addSheetButton.render(parentOffset);
}

} // End namespace AM
