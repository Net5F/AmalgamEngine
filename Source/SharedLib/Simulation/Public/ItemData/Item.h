#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include "ItemInteractionType.h"
#include "ItemProperties.h"
#include "Log.h"
#include "bitsery/ext/std_variant.h"
#include <SDL_stdinc.h>
#include <string>
#include <unordered_map>
#include <array>
#include <vector>

namespace AM
{
/**
 * Holds the data for a single item.
 */
struct Item {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{EngineMessageType::Item};

    /** The max length of a display name. Also the max for string IDs, since 
        they're derived from display name. */
    static constexpr std::size_t MAX_DISPLAY_NAME_LENGTH{50};

    /** The maximum number of interactions that an item can support.
        Note: This is in addition to the Use, Destroy, and Examine interactions
              that all items support. */
    static constexpr std::size_t MAX_INTERACTIONS{4};

    /** The maximum number of interactions that an item can support. */
    static constexpr std::size_t MAX_PROPERTIES{50};

    /** The maximum number of combinations that an item can support. */
    static constexpr std::size_t MAX_COMBINATIONS{50};

    // TODO: Icon ID 

    /** Unique display name, shown in the UI.  */
    std::string displayName{"Null"};

    /** The item's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"null"};

    /** This item's unique numeric identifier.
        This ID will be consistent, and can be used for persistent state. */
    ItemID numericID{NULL_ITEM_ID};

    /** The interactions that this item supports.
        Elements are filled contiguously starting at index 0. Empty elements 
        will be at the end.
        The first interaction in this list is the default interaction.
        Note: This defaults values to ItemInteractionType::NotSet. */
    std::array<ItemInteractionType, MAX_INTERACTIONS> supportedInteractions{};

    /** The properties that are attached to this item.
        Properties hold data that gets used when handling interactions. */
    std::vector<ItemProperty> properties{};

    struct ItemCombination
    {
        /** The item to combine with. */
        ItemID otherItemID{};
        /** The resulting item. */
        ItemID resultItemID{};
    };
    /** A list of the items that this item may be combined with, and the 
        resulting items.
        Note: If you want to put skill requirements on your item combinations, 
              you'll need to build a separate UI/workflow. */
    std::vector<ItemCombination> itemCombinations{};

    /**
     * Finds the first empty index in supportedInteractions and adds the given 
     * interaction.
     * If supportedInteractions is full, prints a warning and does nothing.
     */
    void addInteraction(ItemInteractionType newInteraction);

    /**
     * Returns true if this item supports the given interaction.
     */
    bool containsInteraction(ItemInteractionType desiredInteraction) const;

    /**
     * Adds the given property to this item.
     * If the property already exists, updates the value.
     * 
     * @return false if this item's properties vector is full, else true.
     */
    template<typename T>
    bool addProperty(const T& newProperty)
    {
        // If this item already has the given property, just replace the value.
        for (ItemProperty& property : properties) {
            if (std::holds_alternative<T>(property)) {
                property = newProperty;
                return true;
            }
        }

        // New property, add it.
        if (properties.size() < MAX_PROPERTIES) {
            properties.emplace_back(newProperty);
            return true;
        }
        else {
            LOG_ERROR("Tried to add property to full item.");
            return false;
        }
    }

    /**
     * If this item contains a property of the given type, returns it.
     * Else, returns nullptr.
     */
    template<typename T>
    const ItemProperty* getProperty() const
    {
        // Note: We use a vector and iterate through it every time because 
        //       it reduces wire size (vs a map) and we expect each item to 
        //       only have a few properties.

        // If this item contains the given property type, return it.
        for (const ItemProperty& property : properties) {
            if (std::holds_alternative<T>(property)) {
                return &property;
            }
        }

        return nullptr;
    }
};

template<typename S>
void serialize(S& serializer, Item::ItemCombination& itemCombination)
{
    serializer.value2b(itemCombination.otherItemID);
    serializer.value2b(itemCombination.resultItemID);
}

template<typename S>
void serialize(S& serializer, Item& item)
{
    serializer.text1b(item.displayName, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.text1b(item.stringID, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.value2b(item.numericID);
    serializer.container1b(item.supportedInteractions);
    serializer.container(item.properties, Item::MAX_PROPERTIES,
                         [](S& serializer, ItemProperty& property) {
                             // Note: This calls serialize() for each type.
                             serializer.ext(property,
                                            bitsery::ext::StdVariant{});
                         });
    serializer.container(item.itemCombinations, Item::MAX_COMBINATIONS);
}

} // namespace AM
