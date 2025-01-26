#pragma once

#include "SharedConfig.h"
#include "ItemID.h"
#include "IconID.h"
#include "ItemInteractionType.h"
#include "ItemProperty.h"
#include "ItemCombination.h"
#include "Log.h"
#include "bitsery/ext/std_variant.h"
#include <SDL_stdinc.h>
#include <string>
#include <vector>

namespace AM
{
/**
 * A single item.
 *
 * Items have "interactions", and "properties". Interactions define what users 
 * can do with the item, and properties hold data that can be used while 
 * handling interactions.
 * The engine provides some generic interactions and properties, and more can 
 * be defined by the project.
 *
 * Items are typically defined using a Lua "init script". After that, all 
 * logic is handled in C++. We use this pattern because it provides a good 
 * balance between expressiveness, developer effort, and performance.
 *
 * When an interaction occurs, an event will be pushed into a queue. A system 
 * can then use Simulation::popItemInteractionRequest() to handle the 
 * interaction.
 * Built-in interactions like "Examine", "Combine", and "Use" are handled 
 * automatically by the engine, outside of the queue system.
 * When an item is used on an entity, that entity's "item handler" Lua script 
 * will be ran.
 */
struct Item {
    /** Unique display name, shown in the UI.  */
    std::string displayName{"Null"};

    /** The item's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"null"};

    /** This item's unique numeric identifier.
        This ID will be consistent, and can be used for persistent state. */
    ItemID numericID{NULL_ITEM_ID};

    /** The ID of this item's icon. */
    IconID iconID{NULL_ICON_ID};

    /** How large a stack of this item can be, e.g. in an inventory slot. */
    Uint8 maxStackSize{1};

    /** The item's description text. Used by the Examine interaction. */
    std::string description{};

    /** The interactions that this item supports.
        The first interaction in this list is the default interaction. */
    std::vector<ItemInteractionType> supportedInteractions{};

    /** The properties that are attached to this item.
        Properties hold data that gets used when handling interactions. */
    std::vector<ItemProperty> properties{};

    /** A list of the items that this item may be combined with, and the
        resulting items.
        Note: If you want to put skill requirements on your item combinations,
              you'll need to build a separate UI/workflow. */
    std::vector<ItemCombination> itemCombinations{};

    /** The max length of a display name. Also the max for string IDs, since
        they're derived from display name. */
    static constexpr std::size_t MAX_DISPLAY_NAME_LENGTH{50};

    /** The max length of an item's description text. */
    static constexpr std::size_t MAX_DESCRIPTION_LENGTH{500};

    /** The number of built-in interactions that every item supports: UseOn,
        Destroy, and Examine. */
    static constexpr std::size_t BUILTIN_INTERACTION_COUNT{3};

    /** The total number of interactions that an item can support, including
        the built-ins. */
    static constexpr std::size_t MAX_INTERACTIONS{
        SharedConfig::MAX_ITEM_CUSTOM_INTERACTIONS + BUILTIN_INTERACTION_COUNT};

    /** The maximum number of properties that an item can support. */
    static constexpr std::size_t MAX_PROPERTIES{50};

    /** The maximum number of combinations that an item can support. */
    static constexpr std::size_t MAX_COMBINATIONS{50};

    /**
     * Adds the given interaction to supportedInteractions.
     * @return true if the interaction was added, else false (already present 
     *         or array was full).
     */
    bool addInteraction(ItemInteractionType newInteraction);

    /**
     * Returns true if this item supports the given interaction.
     */
    bool supportsInteraction(ItemInteractionType desiredInteraction) const;

    /**
     * Returns the list of interactions that this item supports, in the order
     * that they should appear in the UI.
     * The list will start with any interactions from supportedInteractions,
     * followed by UseOn, Destroy, and Examine. Any empty slots will be at the
     * end and equal to NotSet.
     */
    std::vector<ItemInteractionType> getInteractionList() const;

    /**
     * Returns this item's default interaction.
     */
    ItemInteractionType getDefaultInteraction() const;

    /**
     * Returns the number of interactions that this item supports.
     * Note: This must be used because the always-present interactions (UseOn, 
     *       Destroy, Examine) aren't stored in supportedInteractions.
     */
    std::size_t getInteractionCount() const;

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
    const T* getProperty() const
    {
        // Note: We use a vector and iterate through it every time because
        //       it reduces wire size (vs a map) and we expect each item to
        //       only have a few properties.

        // If this item contains the given property type, return it.
        for (const ItemProperty& property : properties) {
            if (const T* ptr{std::get_if<T>(&property)}) {
                return ptr;
            }
        }

        return nullptr;
    }
};

template<typename S>
void serialize(S& serializer, Item& item)
{
    serializer.text1b(item.displayName, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.text1b(item.stringID, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.value2b(item.numericID);
    serializer.value2b(item.iconID);
    serializer.value1b(item.maxStackSize);
    serializer.text1b(item.description, Item::MAX_DESCRIPTION_LENGTH);
    serializer.container1b(item.supportedInteractions,
                           SharedConfig::MAX_ITEM_CUSTOM_INTERACTIONS);
    serializer.container(item.properties, Item::MAX_PROPERTIES,
                         [](S& serializer, ItemProperty& property) {
                             // Note: This calls serialize() for each type.
                             serializer.ext(property,
                                            bitsery::ext::StdVariant{});
                         });
    serializer.container(item.itemCombinations, Item::MAX_COMBINATIONS);
}

} // namespace AM
